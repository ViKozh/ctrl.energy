#pragma once

#define CSV_DELIMITER ";"

#define CSV_SUMMARY_DATE_FORMAT "%Y-%m-%d"
#define CSV_EVENTLOG_DATE_FORMAT "%Y-%m-%d %H:%M:%S"

#define CSV_SUMMARY_DATE "date"
#define CSV_SUMMARY_CONSUMPTION_PER_DAY "consumption_per_day"
#define CSV_SUMMARY_CONSUMPTION_TOTAL "consumption_total"
#define CSV_SUMMARY_POWER_FAILURES "power_failures"
#define CSV_SUMMARY_POWER_FAILURES_DURATION "power_failures_duration"
#define CSV_SUMMARY_MIN_VOLTAGE "min_voltage"
#define CSV_SUMMARY_MAX_VOLTAGE "max_voltage"
#define CSV_SUMMARY_UNDERVOLTAGE_FAILURES "undervoltage_failures"
#define CSV_SUMMARY_UNDERVOLTAGE_WARNINGS "undervoltage_warnings"
#define CSV_SUMMARY_OVERVOLTAGE_WARNINGS "overvoltage_warnings"
#define CSV_SUMMARY_OVERVOLTAGE_FAILURES "overvoltage_failures"
#define CSV_SUMMARY_MIN_CURRENT "min_current"
#define CSV_SUMMARY_MAX_CURRENT "max_current"
#define CSV_SUMMARY_OVERLOAD_WARNINGS "overload_warnings"
#define CSV_SUMMARY_OVERLOAD_FAILURES "overload_failures"
#define CSV_SUMMARY_PHASE_IMBALANCE_WARNINGS "phase_imbalance_warnings"
#define CSV_SUMMARY_PHASE_IMBALANCE_FAILURES "phase_imbalance_failures"
#define CSV_SUMMARY_MIN_FREQUENCY "min_frequency"
#define CSV_SUMMARY_MAX_FREQUENCY "max_frequency"
#define CSV_SUMMARY_FREQUENCY_WARNINGS "frequency_warnings"
#define CSV_SUMMARY_FREQUENCY_FAILURES "frequency_failures"
#define CSV_SUMMARY_BREAKER_FAILURES "breaker_failures"
#define CSV_SUMMARY_POWER_METER_FAILURES "power_meter_failures"
#define CSV_SUMMARY_OVERHEATING_WARNINGS "overheating_warnings"
#define CSV_SUMMARY_OVERHEATING_FAILURES "overheating_failures"
#define CSV_SUMMARY_CASE_INTRUSIONS "case_intrusions"

#define CSV_SUMMARY_HEADER                                                                                      \
  CSV_SUMMARY_DATE CSV_DELIMITER CSV_SUMMARY_CONSUMPTION_PER_DAY CSV_DELIMITER CSV_SUMMARY_CONSUMPTION_TOTAL    \
      CSV_DELIMITER CSV_SUMMARY_POWER_FAILURES CSV_DELIMITER CSV_SUMMARY_POWER_FAILURES_DURATION                \
          CSV_DELIMITER CSV_SUMMARY_MIN_VOLTAGE CSV_DELIMITER CSV_SUMMARY_MAX_VOLTAGE                           \
              CSV_DELIMITER CSV_SUMMARY_UNDERVOLTAGE_FAILURES CSV_DELIMITER CSV_SUMMARY_UNDERVOLTAGE_WARNINGS   \
                  CSV_DELIMITER CSV_SUMMARY_OVERVOLTAGE_WARNINGS CSV_DELIMITER CSV_SUMMARY_OVERVOLTAGE_FAILURES \
                      CSV_DELIMITER CSV_SUMMARY_MIN_CURRENT CSV_DELIMITER CSV_SUMMARY_MAX_CURRENT               \
                          CSV_DELIMITER CSV_SUMMARY_OVERLOAD_WARNINGS CSV_DELIMITER                             \
                              CSV_SUMMARY_OVERLOAD_FAILURES CSV_DELIMITER CSV_SUMMARY_PHASE_IMBALANCE_WARNINGS  \
                                  CSV_DELIMITER CSV_SUMMARY_PHASE_IMBALANCE_FAILURES                            \
                                      CSV_DELIMITER CSV_SUMMARY_MIN_FREQUENCY CSV_DELIMITER                     \
                                          CSV_SUMMARY_MAX_FREQUENCY CSV_DELIMITER                               \
                                              CSV_SUMMARY_FREQUENCY_WARNINGS CSV_DELIMITER                      \
                                                  CSV_SUMMARY_FREQUENCY_FAILURES                                \
                                                      CSV_DELIMITER CSV_SUMMARY_BREAKER_FAILURES                \
                                                          CSV_DELIMITER CSV_SUMMARY_POWER_METER_FAILURES        \
                                                              CSV_DELIMITER CSV_SUMMARY_OVERHEATING_WARNINGS    \
                                                                  CSV_DELIMITER                                 \
                                                                      CSV_SUMMARY_OVERHEATING_FAILURES          \
                                                                          CSV_DELIMITER                         \
                                                                              CSV_SUMMARY_CASE_INTRUSIONS

#define CSV_SUMMARY_DATALINE_FORMAT                                            \
  "%s" CSV_DELIMITER "%.3f" CSV_DELIMITER "%.3f" CSV_DELIMITER                 \
  "%d" CSV_DELIMITER "%.2f" CSV_DELIMITER "%.3f" CSV_DELIMITER                 \
  "%.3f" CSV_DELIMITER "%d" CSV_DELIMITER "%d" CSV_DELIMITER                   \
  "%d" CSV_DELIMITER "%d" CSV_DELIMITER "%.3f" CSV_DELIMITER                   \
  "%.3f" CSV_DELIMITER "%d" CSV_DELIMITER "%d" CSV_DELIMITER                   \
  "%d" CSV_DELIMITER "%d" CSV_DELIMITER "%.5f" CSV_DELIMITER                   \
  "%.5f" CSV_DELIMITER "%d" CSV_DELIMITER "%d" CSV_DELIMITER                   \
  "%d" CSV_DELIMITER "%d" CSV_DELIMITER "%d" CSV_DELIMITER "%d" CSV_DELIMITER  \
  "%d\n"

#define CSV_EVENTLOG_TIMESTAMP "timestamp"
#define CSV_EVENTLOG_EVENT_TYPE "event_type"
#define CSV_EVENTLOG_EVENT_CATEGORY "category"
#define CSV_EVENTLOG_MESSAGE "message"

#define CSV_EVENTLOG_HEADER                                                    \
  CSV_EVENTLOG_TIMESTAMP CSV_DELIMITER CSV_EVENTLOG_EVENT_TYPE CSV_DELIMITER   \
      CSV_EVENTLOG_EVENT_CATEGORY CSV_DELIMITER CSV_EVENTLOG_MESSAGE

#define CSV_EVENTLOG_DATALINE_FORMAT                                           \
  "%s" CSV_DELIMITER "%s" CSV_DELIMITER "%s" CSV_DELIMITER "%s\n"
