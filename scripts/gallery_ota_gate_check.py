"""Audit: is the OTA safety story intact? (Phase B invariants)

Three legs, and losing any ONE of them quietly re-opens the failure it closed:
  1. main.cpp must NOT confirm the running image at boot — that disarmed the
     rollback net for any image that merely reached setup().
  2. bl.cpp must confirm it ONLY after a parsed /api/display response
     (markFirmwareValidOnceProven) — an image that cannot poll cannot be
     OTA-rescued, so that is the state rollback must keep covering.
  3. Every FW-Version the device reports must be the per-build wire form
     (Messages::firmware_wire_version) — the server keys OTA offers AND
     convergence on that string, and upstream's dotted version cannot tell
     two gallery builds apart.
Plus, on the board branch only: the sdkconfig must actually arm the
bootloader (scripts/gallery_enable_rollback.py), or legs 1-2 are theatre.
"""
import pathlib, re, sys
fail, seen = [], []

main_cpp = pathlib.Path("src/main.cpp").read_text()
bl = pathlib.Path("src/bl.cpp").read_text()
setup_cpp = pathlib.Path("src/services/device_setup.cpp").read_text()


def strip_comments(s):
    s = re.sub(r"//[^\n]*", "", s)
    return re.sub(r"/\*.*?\*/", "", s, flags=re.S)


# --- 1. no unconditional boot-time confirmation ------------------------------
if "esp_ota_mark_app_valid_cancel_rollback" in strip_comments(main_cpp):
    fail.append("main.cpp confirms the image at boot again — rollback net disarmed")
else:
    seen.append("main.cpp: no boot-time esp_ota_mark_app_valid_cancel_rollback")

# --- 2. the gated confirmation, in the right place ---------------------------
if "static void markFirmwareValidOnceProven(void)" not in bl:
    fail.append("bl.cpp: markFirmwareValidOnceProven definition missing")
m = re.search(r"https_request_err_e handleApiDisplayResponse\(ApiDisplayResponse &apiResponse\)\n\{(.{0,600})", bl, re.S)
if not m:
    fail.append("bl.cpp: could not locate handleApiDisplayResponse body")
elif "markFirmwareValidOnceProven();" not in m.group(1):
    fail.append("bl.cpp: handleApiDisplayResponse no longer calls markFirmwareValidOnceProven")
else:
    seen.append("bl.cpp: image confirmed only on a parsed /api/display response")
n_calls = strip_comments(bl).count("esp_ota_mark_app_valid_cancel_rollback")
if n_calls != 1:
    fail.append(f"esp_ota_mark_app_valid_cancel_rollback appears {n_calls}x in bl.cpp "
                "(must be exactly once, inside the gated helper)")

# --- 3. per-build version on the wire ----------------------------------------
for name, txt in (("bl.cpp", bl), ("services/device_setup.cpp", setup_cpp)):
    body = strip_comments(txt)
    if re.search(r"firmwareVersion\s*=\s*(String\()?FW_VERSION_STRING", body):
        fail.append(f"{name}: reports raw FW_VERSION_STRING — two gallery builds "
                    "become indistinguishable to the OTA server")
    elif "firmwareVersion = Messages::firmware_wire_version()" in body:
        seen.append(f"{name}: reports the per-build wire version")
    else:
        fail.append(f"{name}: firmwareVersion assignment not found")

# --- 4. the bootloader config, where present ---------------------------------
cfg = pathlib.Path("sdkconfigs/sdkconfig.GALLERY_EE03")
if cfg.exists():
    cfg_text = cfg.read_text()
    if "CONFIG_BOOTLOADER_APP_ROLLBACK_ENABLE=y" not in cfg_text:
        fail.append(f"{cfg}: rollback not enabled — run scripts/gallery_enable_rollback.py")
    if re.search(r"^CONFIG_BOOTLOADER_SKIP_VALIDATE_IN_DEEP_SLEEP=y$", cfg_text, re.M):
        fail.append(f"{cfg}: deep-sleep validation skip still set — a failing image "
                    "that sleeps would never roll back; run scripts/gallery_enable_rollback.py")
    if not any(f.startswith(str(cfg)) for f in fail):
        seen.append(f"{cfg}: rollback armed, deep-sleep skip disarmed")
else:
    seen.append(f"{cfg}: absent on this branch (board-branch file) — config leg "
                "checked only after the merge onto gallery/ee03")

print("\n".join("  " + x for x in seen))
print()
print("\n".join(fail) if fail else
      "OK — rollback confirmed only by a proven poll, versions unique per build")
sys.exit(1 if fail else 0)
