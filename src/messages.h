#pragma once
#include <Arduino.h>

namespace Messages {
  String firmware_version();       // display form: "1.8.10 (0d46e7e)"
  String firmware_wire_version();  // wire form:    "1.8.10+0d46e7e"
}
