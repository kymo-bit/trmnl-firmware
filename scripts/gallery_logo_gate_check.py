"""Audit: can a TRMNL logo asset still reach the glass under GALLERY_NO_BOOT_LOGO?

Three routes exist, and each needs a different gate because the two display
entry points differ in NULL-safety:
  display_show_msg()   -- NULL-safe, so pass NULL  (GALLERY_LOGO / storedLogoOrDefault)
  display_show_image() -- derefs immediately, so skip the CALL (#ifndef)
"""
import pathlib, re, sys
fail, seen = [], []

bl = pathlib.Path("src/bl.cpp").read_text()

# --- route 1: the boot splash, via display_show_image -----------------------
n_boot = bl.count("display_show_image(storedLogoOrDefault")
n_guard = bl.count("#ifndef GALLERY_NO_BOOT_LOGO")
seen.append(f"boot splash: {n_boot} call(s), {n_guard} #ifndef guard(s)")
if n_guard < n_boot:
    fail.append(f"boot splash under-guarded: {n_boot} calls, {n_guard} guards")

# --- route 2: every message screen, via storedLogoOrDefault ----------------
m = re.search(r"static uint8_t \*storedLogoOrDefault\(int iType\)\n\{\n(.*?)\n#endif // GALLERY_NO_BOOT_LOGO\n\}",
              bl, re.S)
if not m:
    fail.append("storedLogoOrDefault: could not locate the gated body")
else:
    head = m.group(1).split("#else")[0]
    if "return NULL;" not in head:
        fail.append("storedLogoOrDefault: no NULL return under the flag")
    seen.append(f"storedLogoOrDefault: gated, {bl.count('showMessageWithLogo(')} "
                f"showMessageWithLogo refs route through it")
    body_span = m.span()   # anything inside is behind #else, unreachable

# --- route 3: direct asset refs elsewhere ----------------------------------
for f in sorted(pathlib.Path("src").rglob("*.cpp")):
    t = f.read_text()
    for asset in ("logo_small", "logo_medium", "logo_big"):
        for hit in re.finditer(rf"const_cast<uint8_t \*>\({asset}\)", t):
            if f.name == "bl.cpp" and body_span[0] <= hit.start() <= body_span[1]:
                continue          # inside the gated #else — dead under the flag
            fail.append(f"{f}: ungated const_cast<{asset}> at offset {hit.start()}")
n_macro = sum(p.read_text().count("GALLERY_LOGO(") for p in pathlib.Path("src").rglob("*.cpp"))
seen.append(f"direct asset refs rewritten to GALLERY_LOGO(): {n_macro}")

print("\n".join("  " + x for x in seen))
print()
print("\n".join(fail) if fail else "OK — no logo asset can reach the glass under the flag")
sys.exit(1 if fail else 0)
