#pragma once

#include <string>

#define EMOJI_FAIL "\xF0\x9F\x86\x98"
#define EMOJI_WARN "\xE2\x9A\xA0"
#define EMOJI_OK "\xE2\x9C\x85"
#define EMOJI_LEDGER "\xF0\x9F\x93\x92"
#define EMOJI_LIGHTNING "\xE2\x9A\xA1"

#define TG_SUMMARY_FORMAT EMOJI_LEDGER " -- Daily Summary -- " EMOJI_LEDGER "\n" \
EMOJI_LIGHTNING "<b>%s</b>" EMOJI_LIGHTNING "\n" \
"Power consumed per day: <b>%.2f</b> kWh\n" \
"Total consumption: <b>%.2f</b> kWh\n" \
"Load during day: <b>%.2f kW - %.2f kW approx</b>\n" \
"<i>Quality of service:</i>\n" \
"<blockquote>Stability: %d power failures detected with total duration %d minute(s) %d second(s)\n" \
"Voltage: %.2f V - %.2f V\n" \
"Frequency: %.2f Hz - %.2f Hz</blockquote>\n" \
"<i>Registered events (" EMOJI_WARN " warnings / " EMOJI_FAIL " failures):</i>\n" \
"<blockquote>Undervoltage: %d " EMOJI_WARN " / %d " EMOJI_FAIL "\n" \
"Overvoltage: %d " EMOJI_WARN " / %d " EMOJI_FAIL "\n" \
"Overload: %d " EMOJI_WARN " / %d " EMOJI_FAIL "\n" \
"Phase imbalance: %d " EMOJI_WARN " / %d " EMOJI_FAIL "\n" \
"Frequency shift: %d " EMOJI_WARN " / %d " EMOJI_FAIL "\n" \
"Overheating: %d " EMOJI_WARN " / %d " EMOJI_FAIL "\n" \
"Main Circuit Breaker: %d " EMOJI_FAIL "\n" \
"Power Meter: %d " EMOJI_FAIL "\n" \
"Case Intrusions: %d " EMOJI_FAIL "</blockquote>\n\n" \
"-- provided by ESPHome, <a href=\\\"%s\\\">Home Assistant</a> & <a href=\\\"%s\\\">Grafana</a>."

#define TG_SUMMARY_FORMAT_PART_1 EMOJI_LEDGER " -- Daily Summary [1/3] -- " EMOJI_LEDGER "\n" \
EMOJI_LIGHTNING "<b>%s</b>" EMOJI_LIGHTNING "\n" \
"Power consumed per day: <b>%.2f</b> kWh\n" \
"Total consumption: <b>%.2f</b> kWh\n" \
"Load during day: <b>%.2f kW - %.2f kW approx</b>\n" \
"-- provided by ESPHome, <a href='%s'>Home Assistant</a> & <a href='%s'>Grafana</a>."

#define TG_SUMMARY_FORMAT_PART_2 EMOJI_LEDGER " -- Daily Summary [2/3] -- " EMOJI_LEDGER "\n" \
EMOJI_LIGHTNING "<b>%s</b>" EMOJI_LIGHTNING "\n" \
"<i>Quality of service:</i>\n" \
"<blockquote>Stability: %d power failures detected with total duration %.0f minute(s) %.0f second(s)\n" \
"Voltage: %.2f V - %.2f V\n" \
"Frequency: %.2f Hz - %.2f Hz</blockquote>" 

#define TG_SUMMARY_FORMAT_PART_3 EMOJI_LEDGER " -- Daily Summary [3/3] -- " EMOJI_LEDGER "\n" \
EMOJI_LIGHTNING "<b>%s</b>" EMOJI_LIGHTNING "\n" \
"<i>Registered events (" EMOJI_WARN " warnings / " EMOJI_FAIL " failures):</i>\n" \
"<blockquote>Undervoltage: %d " EMOJI_WARN " / %d " EMOJI_FAIL "\n" \
"Overvoltage: %d " EMOJI_WARN " / %d " EMOJI_FAIL "\n" \
"Overload: %d " EMOJI_WARN " / %d " EMOJI_FAIL "\n" \
"Phase imbalance: %d " EMOJI_WARN " / %d " EMOJI_FAIL "\n" \
"Frequency shift: %d " EMOJI_WARN " / %d " EMOJI_FAIL "\n" \
"Overheating: %d " EMOJI_WARN " / %d " EMOJI_FAIL "\n" \
"Main Circuit Breaker: %d " EMOJI_FAIL "\n" \
"Power Meter: %d " EMOJI_FAIL "\n" \
"Case Intrusions: %d " EMOJI_FAIL "</blockquote>" 


#define TG_FAILURE_MESSAGE_WITH_VALUE EMOJI_FAIL " -- FAILURE: %s [%s] -- " EMOJI_FAIL "\n" \
  "Detected <b><u>CRITICAL</u></b> state: %.2f %s\n" \
  "Total failures today: %d\n"

#define TG_WARNING_MESSAGE_WITH_VALUE EMOJI_WARN " -- WARNING: %s [%s] -- " EMOJI_WARN "\n" \
  "Detected <i>CAUTIOUS</i> state: %.2f %s\n" \
  "Total warnings today: %d\n"

#define TG_RESTORE_MESSAGE_WITH_VALUE EMOJI_OK " -- NORMALIZED: %s [%s] -- " EMOJI_OK "\n" \
  "Current state is: %.2f %s\n" \
  "Total warnings today: %d " EMOJI_WARN "\n" \
  "Total failures today: %d " EMOJI_FAIL "\n"

#define TG_FAILURE_MESSAGE EMOJI_FAIL " -- FAILURE: %s [%s] -- " EMOJI_FAIL "\n" \
  "Detected <b><u>CRITICAL</u></b> state!\n" \
  "Total failures today: %d\n"

#define TG_WARNING_MESSAGE EMOJI_WARN " -- WARNING: %s [%s] -- " EMOJI_WARN "\n" \
  "Detected <i>CAUTIOUS</i> state!\n" \
  "Total warnings today: %d\n"

#define TG_RESTORE_MESSAGE EMOJI_OK " -- NORMALIZED: %s [%s] -- " EMOJI_OK "\n" \
  "Total warnings today: %d " EMOJI_WARN "\n" \
  "Total failures today: %d " EMOJI_FAIL "\n"

#define TG_EMPTY_MESSAGE "  -- UNKNOWN STATE [%s] --  "

#define TG_POWER_FAIL EMOJI_FAIL EMOJI_FAIL EMOJI_FAIL "<b><u>%s</u> energy provider is <u>FAILED</u></b>" EMOJI_FAIL EMOJI_FAIL EMOJI_FAIL "\n" \
  "<i>Additional info</i>:\n" \
  "<blockquote>Circuit Breaker: <b>%s</b>\n" \
  "UPS Battery: <b>%s</b>\n" \
  "Smart Home AC Line: <b>%s</b>\n" \
  "Case: <b>%s</b></blockquote>"

#define TG_POWER_RESTORED EMOJI_OK EMOJI_OK EMOJI_OK "<b><u>%s</u> energy provider is <u>RESTORED</u></b>" EMOJI_OK EMOJI_OK EMOJI_OK "\n" \
  "Total power loss duration:\n" \
  "%d minute(s) %d second(s)"

#define TG_POWER_CONSUMPTION_PER_MONTH EMOJI_LEDGER "<b>Monthly consumption on %s</b>:\n %.2f <b>kW</b>"
