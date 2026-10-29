#pragma once

// GALLERY HEALTH — what a panel tells the server about itself on every poll
// (gallery-app docs/PANEL-HEALTH-2026-10-28.md §4 A). Found 2026-10-28: panel
// No. 1 downloaded six pictures, drew none, and every signal the server had
// called it healthy, because "settled" only meant "handed". These are the
// facts only the panel knows, sent as headers beside the dozen it already
// sends:
//
//   Displayed    the file that last reached the glass
//   Last-Draw    how the last draw went: "ok", or "fail/<what>"; and
//                "ok/ctl-silent" when the display controller did not answer
//                the query made after the refresh
//   Reset-Reason the last reset that was not a wake from deep sleep, and how
//                many such boots there have been: "brownout/3"
namespace GalleryHealth {
  // once per boot, before the first poll
  void noteBoot();
  // "" until a reset has been recorded
  const char *resetReason();

  // the result of the draw just attempted, carried by the NEXT poll. KEPT IN
  // NVS, not RTC memory (found 2026-10-28 on No. 1: a reset, the very thing
  // done to a panel that is not drawing, wiped the result before any poll
  // could carry it); written only when it changes
  void setDraw(const char *result);
  // "" until a draw has ever been attempted
  const char *lastDraw();

  // the file that last reached the glass, kept the same way (DisplayedImage's
  // own record is RTC memory, cleared on every boot on purpose)
  void noteShown(const char *filename);
  // "" until a picture has ever reached the glass
  const char *shown();
} // namespace GalleryHealth
