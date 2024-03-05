#pragma once

#define EMOJI_FAIL "\xF0\x9F\x86\x98"
#define EMOJI_WARN "\xE2\x9A\xA0"
#define EMOJI_OK "\xE2\x9C\x85"
#define EMOJI_LEDGER "\xF0\x9F\x93\x92"
#define EMOJI_LIGHTNING "\xE2\x9A\xA1"

#define TG_SUMMARY_FORMAT EMOJI_LEDGER " -- Daily Summary -- " EMOJI_LEDGER "\n" \
EMOJI_LIGHTNING "*%s*" EMOJI_LIGHTNING "\n" \
"Power consumed per day: *%.2f* kWh\n" \
"Total consumption: *%.2f* kWh\n" \
"Load during day: %.2f kW - %.2f kW approximately\n" \
"_Quality of service:_\n" \
">Stability: %d power failures detected with total duration %d minute(s) %d second(s)\n" \
">Voltage: %.2f V - %.2f V\n" \
">Frequency: %.2f Hz - %.2f Hz**\n" \
"_Registered events (" EMOJI_WARN " warnings / " EMOJI_FAIL " failures):_\n" \
">Undervoltage: %d " EMOJI_WARN " / %d " EMOJI_FAIL "\n" \
">Overvoltage: %d " EMOJI_WARN " / %d " EMOJI_FAIL "\n" \
">Overload: %d " EMOJI_WARN " / %d " EMOJI_FAIL "\n" \
">Phase imbalance: %d " EMOJI_WARN " / %d " EMOJI_FAIL "\n" \
">Frequency shift: %d " EMOJI_WARN " / %d " EMOJI_FAIL "\n" \
">Overheating: %d " EMOJI_WARN " / %d " EMOJI_FAIL "\n" \
">Main Circuit Breaker: %d " EMOJI_FAIL "\n" \
">Power Meter: %d " EMOJI_FAIL "\n" \
">Case Intrusions: %d " EMOJI_FAIL "**\n\n" \
"-- provided by ESPHome, [Home Assistant](%s) & [Grafana](%s)"

#define TG_FAILURE_MESSAGE_WITH_VALUE EMOJI_FAIL " -- FAILURE: %s [%s] -- " EMOJI_FAIL "\n" \
  "Detected *__CRITICAL__* state: %.2f %s\n" \
  "Total failures today: %d\n"

#define TG_WARNING_MESSAGE_WITH_VALUE EMOJI_WARN " -- WARNING: %s [%s] -- " EMOJI_WARN "\n" \
  "Detected _CAUTIOUS_ state: %.2f %s\n" \
  "Total warnings today: %d\n"

#define TG_RESTORE_MESSAGE_WITH_VALUE EMOJI_OK " -- NORMALIZED: %s [%s] -- " EMOJI_OK "\n" \
  "Current state is: %.2f %s\n" \
  "Total warnings today: %d " EMOJI_WARN "\n" \
  "Total failures today: %d " EMOJI_FAIL "\n"

#define TG_FAILURE_MESSAGE EMOJI_FAIL " -- FAILURE: %s [%s] -- " EMOJI_FAIL "\n" \
  "Detected *__CRITICAL__* state!\n" \
  "Total failures today: %d\n"

#define TG_WARNING_MESSAGE EMOJI_WARN " -- WARNING: %s [%s] -- " EMOJI_WARN "\n" \
  "Detected _CAUTIOUS_ state!\n" \
  "Total warnings today: %d\n"

#define TG_RESTORE_MESSAGE EMOJI_OK " -- NORMALIZED: %s [%s] -- " EMOJI_OK "\n" \
  "Total warnings today: %d " EMOJI_WARN "\n" \
  "Total failures today: %d " EMOJI_FAIL "\n"

#define TG_EMPTY_MESSAGE "  -- UNKNOWN STATE [%s] --  "

#define TG_POWER_FAIL EMOJI_FAIL EMOJI_FAIL EMOJI_FAIL "*__%s__ energy provider is __FAILED__*" EMOJI_FAIL EMOJI_FAIL EMOJI_FAIL "\n" \
  "_Additional info_:\n" \
  ">Circuit Breaker: *%s*\n" \
  ">UPS Battery: *%s*\n" \
  ">Smart Home AC Line: *%s*\n" \
  ">Case: *%s*"

#define TG_POWER_RESTORED EMOJI_OK EMOJI_OK EMOJI_OK "*__%s__ energy provider is __RESTORED__*" EMOJI_OK EMOJI_OK EMOJI_OK "\n" \
  "Total power loss duration:\n" \
  "%d minute(s) %d second(s)"

#define TG_POWER_CONSUMPTION_PER_MONTH EMOJI_LEDGER "*Monthly consumption on %s*:\n %.2f *kW*"
