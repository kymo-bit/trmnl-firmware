#include "gallery_health.h"

#include <string.h>
#include <stdio.h>

#include <Preferences.h>

#include "esp_attr.h"
#include "esp_system.h"

namespace GalleryHealth {
  // the same NVS namespace DisplayedImage keeps `painted` in
  static const char *NS = "gallery";
  static const char *RESET = "rst";
  static const char *RESETS = "rstn";
  static const char *DRAW = "draw";
  static const char *SHOWN = "shown";

  static char draw[32] = {0};
  static char shownName[36] = {0};
  static char reset[32] = {0};
  static bool loaded = false;

  // read once per boot; NVS keeps them through resets and power cuts
  static void load() {
    if (loaded) return;
    loaded = true;
    Preferences p;
    if (!p.begin(NS, true)) return;
    strncpy(draw, p.getString(DRAW, "").c_str(), sizeof(draw) - 1);
    strncpy(shownName, p.getString(SHOWN, "").c_str(), sizeof(shownName) - 1);
    p.end();
  }

  // the value, written only when it changed
  static void keep(const char *key, char *buf, size_t size, const char *value) {
    load();
    if (strncmp(buf, value, size - 1) == 0) return;
    strncpy(buf, value, size - 1);
    buf[size - 1] = '\0';
    Preferences p;
    if (!p.begin(NS, false)) return;
    p.putString(key, buf);
    p.end();
  }

  static const char *name(esp_reset_reason_t r) {
    switch (r) {
      case ESP_RST_POWERON: return "poweron";
      case ESP_RST_EXT: return "external";
      case ESP_RST_SW: return "software";
      case ESP_RST_PANIC: return "panic";
      case ESP_RST_INT_WDT: return "int-wdt";
      case ESP_RST_TASK_WDT: return "task-wdt";
      case ESP_RST_WDT: return "wdt";
      case ESP_RST_BROWNOUT: return "brownout";
      case ESP_RST_SDIO: return "sdio";
      default: return "unknown";
    }
  }

  void noteBoot() {
    esp_reset_reason_t r = esp_reset_reason();
    Preferences p;
    if (!p.begin(NS, r == ESP_RST_DEEPSLEEP)) return;
    if (r != ESP_RST_DEEPSLEEP) {
      // a real reset: count it and keep its kind. NVS is written only here,
      // never on the deep-sleep wakes that are every other boot
      uint32_t n = p.getUInt(RESETS, 0) + 1;
      p.putUInt(RESETS, n);
      p.putString(RESET, name(r));
    }
    String last = p.getString(RESET, "");
    uint32_t n = p.getUInt(RESETS, 0);
    p.end();
    if (last.length()) snprintf(reset, sizeof(reset), "%s/%lu", last.c_str(), (unsigned long)n);
  }

  const char *resetReason() { return reset; }

  void setDraw(const char *result) { keep(DRAW, draw, sizeof(draw), result); }

  const char *lastDraw() {
    load();
    return draw;
  }

  void noteShown(const char *filename) { keep(SHOWN, shownName, sizeof(shownName), filename); }

  const char *shown() {
    load();
    return shownName;
  }
}
