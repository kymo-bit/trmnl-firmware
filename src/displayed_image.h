#pragma once

// Tracks which image is physically on the display, surviving deep sleep,
// so the next wakeup can skip repainting when the server re-serves it.
namespace DisplayedImage {
  void remember(const char *filename);
  void clear();
  bool exists();
  bool matches(const char *filename);
  char *get();

  // Has ANY image ever been painted on this glass?
  //
  // Distinct from exists(), and the difference is the whole point.
  // szPrevFile is RTC_DATA_ATTR: it survives deep sleep but NOT a power cycle,
  // and the boot path deliberately calls clear(). So after a reboot exists()
  // is false while E-PAPER IS STILL HOLDING THE PICTURE. Anything that asks
  // "is the glass blank?" in order to decide whether it may paint over art
  // must ask THIS, which is in NVS and outlives a reboot.
  bool everPainted();
} // namespace DisplayedImage
