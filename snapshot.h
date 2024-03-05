#pragma once

#include "csv_strings.h"
#include "problems.h"
#include "sdcard.h"
#include "settings.h"
#include "tg_bot_strings.h"
#include <esphome/core/util.h>


#define TAG_SNAPSHOT "Snapshot"

#define SNAPSHOT_FILE "snapshot.dat"

#define SNAPLOG_FILE "datalog_.csv"
#define SNAPLOG_MAX_SIZE 16777216
#define SNAPLOG_ROTATE_FILE "data%.4d.csv"

#define REASON_TIMER "REGULAR"
#define REASON_FAILURE "FAIL"
#define REASON_RESTORE "RESTORE"

#define CURRENT_VERSION 1

/// @brief Snapshot data slice
struct SnapSlice {
  double energyConsumption;
  bool isUnpowered;
  uint32_t powerFailuresCount;
  uint64_t powerFailuresDuration;
  double minVoltage;
  uint32_t undervoltageFailures;
  uint64_t undervoltageWarnings;
  double maxVoltage;
  uint32_t overvoltageFailures;
  uint64_t overvoltageWarnings;
  double minCurrent;
  double maxCurrent;
  uint32_t overloadFailures;
  uint64_t overloadWarnings;
  uint32_t phaseImbalanceFailures;
  uint64_t phaseImbalanceWarnings;
  double minFrequency;
  double maxFrequency;
  uint32_t frequencyFailures;
  uint64_t frequencyWarnings;
  uint64_t breakerFailures;
  uint64_t powerMeterFailures;
  uint64_t caseIntrusionFailures;
  uint64_t overheatingWarnings;
  uint64_t overheatingFailures;

  SnapSlice operator+(SnapSlice other) {
    return SnapSlice{
        energyConsumption = this->energyConsumption + other.energyConsumption,
        isUnpowered = this->isUnpowered || other.isUnpowered,
        powerFailuresCount =
            this->powerFailuresCount + other.powerFailuresCount,
        powerFailuresDuration =
            this->powerFailuresDuration + other.powerFailuresDuration,
        minVoltage = min(this->minVoltage, other.minVoltage),
        undervoltageFailures =
            this->undervoltageFailures + other.undervoltageFailures,
        undervoltageWarnings =
            this->undervoltageWarnings + other.undervoltageWarnings,
        maxVoltage = max(this->maxVoltage, other.maxVoltage),
        overvoltageFailures =
            this->overvoltageFailures + other.overvoltageFailures,
        overvoltageWarnings =
            this->overvoltageWarnings + other.overvoltageWarnings,
        minCurrent = min(this->minCurrent, other.minCurrent),
        maxCurrent = max(this->maxCurrent, other.maxCurrent),
        overloadFailures = this->overloadFailures + other.overloadFailures,
        overloadWarnings = this->overloadWarnings + other.overloadWarnings,
        phaseImbalanceFailures =
            this->phaseImbalanceFailures + other.phaseImbalanceFailures,
        phaseImbalanceWarnings =
            this->phaseImbalanceWarnings + other.phaseImbalanceWarnings,
        minFrequency = min(this->minFrequency, other.minFrequency),
        maxFrequency = max(this->maxFrequency, other.maxFrequency),
        frequencyFailures = this->frequencyFailures + other.frequencyFailures,
        frequencyWarnings = this->frequencyWarnings + other.frequencyWarnings,
        breakerFailures = this->breakerFailures + other.breakerFailures,
        powerMeterFailures =
            this->powerMeterFailures + other.powerMeterFailures,
        caseIntrusionFailures =
            this->caseIntrusionFailures + other.caseIntrusionFailures,
        overheatingWarnings =
            this->overheatingWarnings + other.overheatingWarnings,
        overheatingFailures =
            this->overheatingFailures + other.overheatingFailures};
  }
};

struct Snapshot {
  int updateTimestamp;
  uint32_t version;
  SnapSlice totalPrevDaysData;
  SnapSlice dailyData;
  int activePowerFailureStartTS;
  int activePowerFailureShiftingStartTS;
  int activePowerFailureEndTS;
  int lastPowerFailureDuration;
};

struct SnapshotStorage {
  uint16_t crc16;
  union {
    Snapshot dataset;
    uint8_t binary[sizeof(dataset)];
  };
};

union SnapshotData {
  SnapshotStorage content;
  uint8_t data[sizeof(SnapshotStorage)];
};

static SnapshotData snapData;

//static double prevDayConsumption;

bool writeDailyLogCSVHeader(fs::File &file) {
  if (!file)
    return false;
  return file.println(CSV_SUMMARY_HEADER) != 0;
};

bool writeDailyLogCSVDataLine(fs::File &file, esphome::ESPTime time) {
  if (!time.is_valid() || !file)
    return false;

  double totalConsumption =
      snapData.content.dataset.dailyData.energyConsumption +
      snapData.content.dataset.totalPrevDaysData.energyConsumption;

  return file.printf(
             CSV_SUMMARY_DATALINE_FORMAT,
             time.strftime(CSV_SUMMARY_DATE_FORMAT),
             snapData.content.dataset.dailyData.energyConsumption,
             totalConsumption,
             snapData.content.dataset.dailyData.powerFailuresCount,
             snapData.content.dataset.dailyData.powerFailuresDuration / 60,
             snapData.content.dataset.dailyData.minVoltage,
             snapData.content.dataset.dailyData.maxVoltage,
             snapData.content.dataset.dailyData.undervoltageFailures,
             snapData.content.dataset.dailyData.undervoltageWarnings,
             snapData.content.dataset.dailyData.overvoltageWarnings,
             snapData.content.dataset.dailyData.overvoltageFailures,
             snapData.content.dataset.dailyData.minCurrent,
             snapData.content.dataset.dailyData.maxCurrent,
             snapData.content.dataset.dailyData.overloadWarnings,
             snapData.content.dataset.dailyData.overloadFailures,
             snapData.content.dataset.dailyData.phaseImbalanceWarnings,
             snapData.content.dataset.dailyData.phaseImbalanceFailures,
             snapData.content.dataset.dailyData.minFrequency,
             snapData.content.dataset.dailyData.maxFrequency,
             snapData.content.dataset.dailyData.frequencyWarnings,
             snapData.content.dataset.dailyData.frequencyFailures,
             snapData.content.dataset.dailyData.breakerFailures,
             snapData.content.dataset.dailyData.powerMeterFailures,
             snapData.content.dataset.dailyData.overheatingWarnings,
             snapData.content.dataset.dailyData.overheatingFailures,
             snapData.content.dataset.dailyData.caseIntrusionFailures) != 0;
};

bool writeDailyLog(esphome::ESPTime time) {
  if (SD.cardType() == CARD_NONE) {
    ESP_LOGW(TAG_SNAPSHOT, "There's no SD card to write down summary data.");
    return false;
  };

  if (!sdcard::ensure_date_dir_path(
          time, sdcard::DateDirectoryMode::BY_YEAR_THEN_BY_MONTH)) {
    ESP_LOGE(TAG_SNAPSHOT,
             "Unable to initialize date-dependent directory tree.");
    return false;
  }

  if (!sdcard::claim()) {
    ESP_LOGE(TAG_SETTINGS,
             "Unable to open file. Another file is opened already.");
    return false;
  }

  auto filename = sdcard::generate_date_dir_path(
      time, sdcard::DateDirectoryMode::BY_YEAR_THEN_BY_MONTH, SNAPLOG_FILE);
  bool need_header = !SD.exists(filename);
  fs::File file = SD.open(filename, FILE_APPEND, true);
  if (!file) {
    ESP_LOGE(TAG_SNAPSHOT, "Unable to create or open data log file.");
    sdcard::free();
    return false;
  };

  bool is_success = true;
  if (need_header)
    is_success &= writeDailyLogCSVHeader(file);
  is_success &= writeDailyLogCSVDataLine(file, time);
  file.close();
  sdcard::free();
  if (!is_success) {
    ESP_LOGW(TAG_SNAPSHOT, "Unable to write down a data log record.");
    return false;
  };

  return true;
};

std::string generateTelegramBotSummaryMessage(const char *source_name,
                                              const char *ha_uri,
                                              const char *grafana_uri) {
  double totalConsumption =
      snapData.content.dataset.dailyData.energyConsumption +
      snapData.content.dataset.totalPrevDaysData.energyConsumption;
  int power_loss_minutes = static_cast<int>(
      snapData.content.dataset.dailyData.powerFailuresDuration / 60);
  int power_loss_seconds =
      snapData.content.dataset.dailyData.powerFailuresDuration -
      power_loss_minutes * 60;
  char buffer[2048]; // Max message size
  snprintf(
      buffer, sizeof(buffer), TG_SUMMARY_FORMAT, source_name,
      snapData.content.dataset.dailyData.energyConsumption, totalConsumption,
      snapData.content.dataset.dailyData.minCurrent * VOLTAGE_LEVEL / 1000,
      snapData.content.dataset.dailyData.maxCurrent * VOLTAGE_LEVEL / 1000,
      snapData.content.dataset.dailyData.powerFailuresCount, power_loss_minutes,
      power_loss_seconds, snapData.content.dataset.dailyData.minVoltage,
      snapData.content.dataset.dailyData.maxVoltage,
      snapData.content.dataset.dailyData.minFrequency,
      snapData.content.dataset.dailyData.maxFrequency,
      snapData.content.dataset.dailyData.undervoltageWarnings,
      snapData.content.dataset.dailyData.undervoltageFailures,
      snapData.content.dataset.dailyData.overvoltageWarnings,
      snapData.content.dataset.dailyData.overvoltageFailures,
      snapData.content.dataset.dailyData.overloadWarnings,
      snapData.content.dataset.dailyData.overloadFailures,
      snapData.content.dataset.dailyData.phaseImbalanceWarnings,
      snapData.content.dataset.dailyData.phaseImbalanceFailures,
      snapData.content.dataset.dailyData.frequencyWarnings,
      snapData.content.dataset.dailyData.frequencyFailures,
      snapData.content.dataset.dailyData.overheatingWarnings,
      snapData.content.dataset.dailyData.overheatingFailures,
      snapData.content.dataset.dailyData.breakerFailures,
      snapData.content.dataset.dailyData.powerMeterFailures,
      snapData.content.dataset.dailyData.caseIntrusionFailures,
      ha_uri, grafana_uri);
  return std::string(buffer);
};

void saveToSnapshot(double currentConsumption) {
  snapData.content.dataset.dailyData.energyConsumption =
      currentConsumption -
      snapData.content.dataset.totalPrevDaysData.energyConsumption;
  snapData.content.dataset.activePowerFailureShiftingStartTS = 
    powerFailureShiftingStartTS;
  snapData.content.dataset.lastPowerFailureDuration =
    lastPowerFailureDuration;
  snapData.content.dataset.dailyData.powerFailuresCount =
      dailyFailures[Problems::GENERIC_POWER_FAILURE];
  snapData.content.dataset.dailyData.powerFailuresDuration =
      dailyPowerFailureDuration;
  if (getProblem(Problems::GENERIC_POWER_FAILURE) == ProblemState::FAILURE) {
    snapData.content.dataset.activePowerFailureStartTS = powerFailureStartTS;
    snapData.content.dataset.activePowerFailureEndTS = 0;
  } else {
    snapData.content.dataset.activePowerFailureStartTS = powerFailureStartTS;
    snapData.content.dataset.activePowerFailureEndTS = powerFailureEndTS;
  };
  snapData.content.dataset.dailyData.minVoltage = minVoltage;
  snapData.content.dataset.dailyData.undervoltageFailures =
      dailyFailures[Problems::UNDERVOLTAGE];
  snapData.content.dataset.dailyData.undervoltageWarnings =
      dailyWarnings[Problems::UNDERVOLTAGE];
  snapData.content.dataset.dailyData.maxVoltage = maxVoltage;
  snapData.content.dataset.dailyData.overvoltageFailures =
      dailyFailures[Problems::UNDERVOLTAGE];
  snapData.content.dataset.dailyData.overvoltageWarnings =
      dailyWarnings[Problems::UNDERVOLTAGE];
  snapData.content.dataset.dailyData.minCurrent = minCurrent;
  snapData.content.dataset.dailyData.maxCurrent = maxCurrent;
  snapData.content.dataset.dailyData.overloadFailures =
      dailyFailures[Problems::OVERLOAD];
  snapData.content.dataset.dailyData.overloadWarnings =
      dailyWarnings[Problems::OVERLOAD];
  snapData.content.dataset.dailyData.phaseImbalanceFailures =
      dailyFailures[Problems::PHASE_SHIFT];
  snapData.content.dataset.dailyData.phaseImbalanceWarnings =
      dailyWarnings[Problems::PHASE_SHIFT];
  snapData.content.dataset.dailyData.minFrequency = minFrequency;
  snapData.content.dataset.dailyData.maxFrequency = maxFrequency;
  snapData.content.dataset.dailyData.frequencyFailures =
      dailyFailures[Problems::FREQUENCY_SHIFT];
  snapData.content.dataset.dailyData.frequencyWarnings =
      dailyWarnings[Problems::FREQUENCY_SHIFT];
  snapData.content.dataset.dailyData.breakerFailures =
      dailyFailures[Problems::BREAKER];
  snapData.content.dataset.dailyData.powerMeterFailures =
      dailyFailures[Problems::POWER_METER];
  snapData.content.dataset.dailyData.caseIntrusionFailures =
      dailyFailures[Problems::INTRUSION];
  snapData.content.dataset.dailyData.overheatingFailures =
      dailyFailures[Problems::OVERHEAT];
  snapData.content.dataset.dailyData.overheatingWarnings =
      dailyWarnings[Problems::OVERHEAT];
};

void loadFromSnapshot(double currentConsumption) {
  // snapData.content.dataset.dailyData.energyConsumption = currentConsumption -
  //     snapData.content.dataset.totalPrevDaysData.energyConsumption;
  dailyFailures[Problems::GENERIC_POWER_FAILURE] =
      snapData.content.dataset.dailyData.powerFailuresCount;
  dailyPowerFailureDuration =
      snapData.content.dataset.dailyData.powerFailuresDuration;
  if (getProblem(Problems::GENERIC_POWER_FAILURE) == ProblemState::FAILURE) {
    powerFailureStartTS = snapData.content.dataset.activePowerFailureStartTS;
    powerFailureEndTS = 0;
  } else {
    powerFailureStartTS = snapData.content.dataset.activePowerFailureStartTS;
    powerFailureEndTS = snapData.content.dataset.activePowerFailureEndTS;
  };
  powerFailureShiftingStartTS = snapData.content.dataset.activePowerFailureShiftingStartTS;
  lastPowerFailureDuration = snapData.content.dataset.lastPowerFailureDuration;
  minVoltage = min(minVoltage, snapData.content.dataset.dailyData.minVoltage);
  dailyFailures[Problems::UNDERVOLTAGE] =
      snapData.content.dataset.dailyData.undervoltageFailures;
  dailyWarnings[Problems::UNDERVOLTAGE] =
      snapData.content.dataset.dailyData.undervoltageWarnings;
  maxVoltage = max(snapData.content.dataset.dailyData.maxVoltage, maxVoltage);
  dailyFailures[Problems::UNDERVOLTAGE] =
      snapData.content.dataset.dailyData.overvoltageFailures;
  dailyWarnings[Problems::UNDERVOLTAGE] =
      snapData.content.dataset.dailyData.overvoltageWarnings;
  minCurrent = min(snapData.content.dataset.dailyData.minCurrent, minCurrent);
  maxCurrent = max(snapData.content.dataset.dailyData.maxCurrent, maxCurrent);
  dailyFailures[Problems::OVERLOAD] =
      snapData.content.dataset.dailyData.overloadFailures;
  dailyWarnings[Problems::OVERLOAD] =
      snapData.content.dataset.dailyData.overloadWarnings;
  dailyFailures[Problems::PHASE_SHIFT] =
      snapData.content.dataset.dailyData.phaseImbalanceFailures;
  dailyWarnings[Problems::PHASE_SHIFT] =
      snapData.content.dataset.dailyData.phaseImbalanceWarnings;
  minFrequency =
      min(snapData.content.dataset.dailyData.minFrequency, minFrequency);
  maxFrequency =
      max(snapData.content.dataset.dailyData.maxFrequency, maxFrequency);
  dailyFailures[Problems::FREQUENCY_SHIFT] =
      snapData.content.dataset.dailyData.frequencyFailures;
  dailyWarnings[Problems::FREQUENCY_SHIFT] =
      snapData.content.dataset.dailyData.frequencyWarnings;
  dailyFailures[Problems::BREAKER] =
      snapData.content.dataset.dailyData.breakerFailures;
  dailyFailures[Problems::POWER_METER] =
      snapData.content.dataset.dailyData.powerMeterFailures;
  dailyFailures[Problems::INTRUSION] =
      snapData.content.dataset.dailyData.caseIntrusionFailures;
  dailyFailures[Problems::OVERHEAT] =
      snapData.content.dataset.dailyData.overheatingFailures;
  dailyWarnings[Problems::OVERHEAT] =
      snapData.content.dataset.dailyData.overheatingWarnings;
};

/// @brief Resets daily counters
void commitDailyData(double currentConsumption, int currentTimestamp) {
  snapData.content.dataset.totalPrevDaysData =
      snapData.content.dataset.dailyData +
      snapData.content.dataset.totalPrevDaysData;
  if (snapData.content.dataset.totalPrevDaysData.energyConsumption <
      currentConsumption) {
    snapData.content.dataset.totalPrevDaysData.energyConsumption =
        currentConsumption;
  };
  if (getProblem(Problems::GENERIC_POWER_FAILURE) == ProblemState::FAILURE) {
    snapData.content.dataset.totalPrevDaysData.powerFailuresDuration +=
        currentTimestamp - powerFailureStartTS;
    snapData.content.dataset.activePowerFailureStartTS = currentTimestamp;
    snapData.content.dataset.activePowerFailureEndTS = 0;
    snapData.content.dataset.dailyData.powerFailuresCount = 1;
  } else {
    snapData.content.dataset.activePowerFailureStartTS = powerFailureStartTS;
    snapData.content.dataset.activePowerFailureEndTS = powerFailureEndTS;
    snapData.content.dataset.dailyData.powerFailuresCount = 0;
  };
  snapData.content.dataset.dailyData.energyConsumption = 0;
  snapData.content.dataset.dailyData.powerFailuresDuration = 0;
  snapData.content.dataset.dailyData.minVoltage = VOLTAGE_LEVEL;
  snapData.content.dataset.dailyData.undervoltageFailures = 0;
  snapData.content.dataset.dailyData.undervoltageWarnings = 0;
  snapData.content.dataset.dailyData.maxVoltage = 0;
  snapData.content.dataset.dailyData.overvoltageFailures = 0;
  snapData.content.dataset.dailyData.overvoltageWarnings = 0;
  snapData.content.dataset.dailyData.minCurrent = SUPPORTED_LOAD_LEVEL;
  snapData.content.dataset.dailyData.maxCurrent = 0;
  snapData.content.dataset.dailyData.overloadFailures = 0;
  snapData.content.dataset.dailyData.overloadWarnings = 0;
  snapData.content.dataset.dailyData.phaseImbalanceFailures = 0;
  snapData.content.dataset.dailyData.phaseImbalanceWarnings = 0;
  snapData.content.dataset.dailyData.minFrequency = FREQUENCY;
  snapData.content.dataset.dailyData.maxFrequency = 0;
  snapData.content.dataset.dailyData.frequencyFailures = 0;
  snapData.content.dataset.dailyData.frequencyWarnings = 0;
  snapData.content.dataset.dailyData.breakerFailures = 0;
  snapData.content.dataset.dailyData.powerMeterFailures = 0;
  snapData.content.dataset.dailyData.caseIntrusionFailures = 0;
  snapData.content.dataset.dailyData.overheatingWarnings = 0;
  snapData.content.dataset.dailyData.overheatingFailures = 0;
};
