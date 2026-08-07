#include "messages.h"

#include "config.h"

namespace Messages {
  String firmware_version() {
    if (FW_COMMIT[0] != '\0') {
      return String(FW_VERSION_STRING) + " (" + String(FW_COMMIT) + ")";
    } else {
      return FW_VERSION_STRING;
    }
  }

  // What goes in the FW-Version header — UNIQUE PER BUILD. The gallery server
  // offers an OTA to any panel whose reported version differs from the
  // published target and treats "reported == declared" as convergence, so the
  // dotted upstream version alone would make every gallery build look
  // identical to it. '+' (semver build metadata), not the display form above:
  // this string is compared verbatim and appears in a URL path server-side,
  // where spaces and parens are unwelcome.
  String firmware_wire_version() {
    if (FW_COMMIT[0] != '\0') {
      return String(FW_VERSION_STRING) + "+" + String(FW_COMMIT);
    } else {
      return FW_VERSION_STRING;
    }
  }
} // namespace Messages
