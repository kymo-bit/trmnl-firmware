"""Idempotently arm the OTA rollback net in the GALLERY_EE03 sdkconfig.

    python3 scripts/gallery_enable_rollback.py

Run on `gallery/ee03` AFTER merging the gallery branch (the sdkconfig lives
only on the board branch). Safe to run every deploy; prints OK when nothing
needed changing, CHANGED when it edited the file — commit the edit in that
case, exactly like the build-flag step in FLASH-DEPLOY-EE03.md §2.

Two settings, both required, and the second is the one that would silently
defeat the first:

  CONFIG_BOOTLOADER_APP_ROLLBACK_ENABLE=y
      A freshly OTA'd image boots PENDING_VERIFY; unless the app confirms it
      (bl.cpp does so only after a parsed /api/display response), the next
      full boot reverts to the previous image. Without this, "rollback" is a
      no-op and a broken-but-booting OTA build keeps the wall.

  CONFIG_BOOTLOADER_SKIP_VALIDATE_IN_DEEP_SLEEP unset
      With it set, a deep-sleep wake bypasses the bootloader's otadata check
      — and this panel's whole life is deep-sleep wakes. A bad image that
      polls, fails, and sleeps would keep waking into itself forever, never
      giving the bootloader a chance to revert. The cost of unsetting it is a
      slower wake (full image verification); a wall that cannot brick is
      worth hundreds of milliseconds.
"""
import pathlib
import re
import sys

CFG = pathlib.Path("sdkconfigs/sdkconfig.GALLERY_EE03")

if not CFG.exists():
    print(f"FAIL  {CFG} not found — run from the repo root, on gallery/ee03 "
          "(the board branch; the gallery feature branch does not carry it)")
    sys.exit(1)

text = orig = CFG.read_text()

# enable rollback: flip the "not set" comment, or append if absent entirely
if "CONFIG_BOOTLOADER_APP_ROLLBACK_ENABLE=y" not in text:
    text, n = re.subn(
        r"^# CONFIG_BOOTLOADER_APP_ROLLBACK_ENABLE is not set$",
        "CONFIG_BOOTLOADER_APP_ROLLBACK_ENABLE=y",
        text, flags=re.M)
    if n == 0:
        text += "\nCONFIG_BOOTLOADER_APP_ROLLBACK_ENABLE=y\n"

# disarm the deep-sleep validation skip
text = re.sub(
    r"^CONFIG_BOOTLOADER_SKIP_VALIDATE_IN_DEEP_SLEEP=y$",
    "# CONFIG_BOOTLOADER_SKIP_VALIDATE_IN_DEEP_SLEEP is not set",
    text, flags=re.M)

if text != orig:
    CFG.write_text(text)
    print(f"CHANGED  {CFG} — commit this (rollback armed, deep-sleep "
          "validation skip disarmed)")
else:
    print(f"OK  {CFG} already armed")
sys.exit(0)
