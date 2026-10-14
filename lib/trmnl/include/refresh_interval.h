#pragma once

#include <persistence_interface.h>
#include <stdint.h>

// Single owner of the device's stored refresh interval ("refresh_rate" in NVS):
// the number of seconds goToSleep() arms the deep-sleep timer with. Covers the
// server-provided rate, the fast-poll backoff used while waiting for setup or
// content, the API/WiFi retry ladders, and the fixed fallback.
class RefreshInterval {
public:
  static constexpr const char *SLEEP_KEY = "refresh_rate";
  static constexpr const char *STREAK_KEY = "fast_polls";

  static constexpr uint32_t DEFAULT_SECONDS = 900;

  // Bounds on anything the SERVER asks for. The stored interval is what
  // goToSleep() arms the timer with, so an out-of-range value is not a bad
  // setting — it is a device that stops being a device.
  //
  // MAX: the rate arrives as an unsigned, so a negative or garbage field wraps
  // to an enormous number and arms a timer years out. A panel sealed behind a
  // mat has no reachable button, so that is indistinguishable from dead.
  // MIN: a mistyped or stale fast rate is how this wall flattened six cells
  // twice (RUNBOOK, the drain row). The floor costs nothing while live widgets
  // are off, and if they are ever turned on this is the knob to revisit.
  static constexpr uint32_t MIN_SERVER_SECONDS = 60;
  static constexpr uint32_t MAX_SERVER_SECONDS = 86400;
  static constexpr uint32_t WIFI_OUTAGE_SECONDS = 3600;

  explicit RefreshInterval(Persistence &persistence);

  // Stored interval in seconds; returns DEFAULT_SECONDS (or defaultValue) when unset.
  uint32_t seconds();
  uint32_t seconds(uint32_t defaultValue);

  // Each apply* method stores its interval and returns the seconds stored.

  // Server-provided rate from /api/display.
  uint32_t applyServerRate(uint32_t rate);

  // Fast refresh with 5-second sleeps for quick updates during initial device
  // setup, backing off in case the server never returns a good response.
  uint32_t applyFastPoll();
  void resetFastPollStreak();

  // Retry ladders; attempt is 1-based.
  uint32_t applyApiRetry(uint8_t attempt);  // 15 / 30 / 60, then DEFAULT_SECONDS
  uint32_t applyWifiRetry(uint8_t attempt); // 60 / 180, then 300
  // The ladder is exhausted and the network is simply not there. Long enough
  // that a multi-day outage cannot flatten a pack, short enough that the panel
  // is back the day the network is. Replaces a button-only sleep that a
  // mat-mounted panel could never wake from.
  uint32_t applyWifiOutage();               // 3600
  uint32_t applyDefault();                  // fixed fallback, e.g. /api/setup 404

private:
  static uint32_t fastPollSeconds(uint32_t streak);
  bool writeIfChanged(uint32_t value);

  Persistence &persistence;
};
