#include "displayed_image.h"

#include <string.h>

#include <Preferences.h>

#include "esp_attr.h"

namespace DisplayedImage {
    // SPIFFS path of the image currently on the display
  RTC_DATA_ATTR static char szPrevFile[36] = {0};

    // filename must already be a 32-byte fixed SPIFFS path (see Storage::fix_file_name)
  // NVS, not RTC memory: this one has to survive a power cycle, because the
  // picture on the glass does.
  static const char *NS = "gallery";
  static const char *PAINTED = "painted";

  void remember(const char *filename) {
    strncpy(szPrevFile, filename, sizeof(szPrevFile) - 1);
    szPrevFile[sizeof(szPrevFile) - 1] = '\0';
    if (szPrevFile[0] != '\0' && !everPainted()) {
      Preferences p;
      if (p.begin(NS, false)) {
        p.putBool(PAINTED, true);
        p.end();
      }
    }
  }

  bool everPainted() {
    Preferences p;
    if (!p.begin(NS, true)) return false;
    bool v = p.getBool(PAINTED, false);
    p.end();
    return v;
  }

  void clear() { memset(szPrevFile, 0, sizeof(szPrevFile)); }

  bool exists() { return szPrevFile[0] != '\0'; }

  bool matches(const char *filename) { return strcmp(szPrevFile, filename) == 0; }

  char *get() { return szPrevFile; }
} // namespace DisplayedImage
