#pragma once

#include "csv_strings.h"
#include "problems.h"
#include "sdcard.h"
#include "settings.h"
#include "tg_bot_strings.h"
#include <esphome/core/util.h>

#define TAG_SNAPSHOT "Snapshot"

#define SNAPSHOT_FILE "/snapshot.dat"

#define SNAPLOG_FILE "datalog_.csv"
#define SNAPLOG_MAX_SIZE 16777216
#define SNAPLOG_ROTATE_FILE "data%.4d.csv"

#define REASON_TIMER "REGULAR"
#define REASON_FAILURE "FAIL"
#define REASON_RESTORE "RESTORE"

#define SNAPSHOT_DATA_VERSION 2

#pragma pack(0)

/// @brief Snapshot data slice
struct SnapSlice
{
    uint64_t powerFailuresCount;
    uint64_t powerFailuresDuration;
    uint64_t undervoltageFailures;
    uint64_t undervoltageWarnings;
    uint64_t overvoltageFailures;
    uint64_t overvoltageWarnings;
    uint64_t overloadFailures;
    uint64_t overloadWarnings;
    uint64_t phaseImbalanceFailures;
    uint64_t phaseImbalanceWarnings;
    uint64_t frequencyFailures;
    uint64_t frequencyWarnings;
    uint64_t breakerFailures;
    uint64_t powerMeterFailures;
    uint64_t caseIntrusionFailures;
    uint64_t overheatingWarnings;
    uint64_t overheatingFailures;
    double_t minFrequency;
    double_t maxFrequency;
    double_t minCurrent;
    double_t maxCurrent;
    double_t minVoltage;
    double_t maxVoltage;
    double_t energyConsumption;

    SnapSlice operator+(SnapSlice other)
    {
        return SnapSlice{
            powerFailuresCount =
                this->powerFailuresCount + other.powerFailuresCount,
            powerFailuresDuration =
                this->powerFailuresDuration + other.powerFailuresDuration,
            undervoltageFailures =
                this->undervoltageFailures + other.undervoltageFailures,
            undervoltageWarnings =
                this->undervoltageWarnings + other.undervoltageWarnings,
            overvoltageFailures =
                this->overvoltageFailures + other.overvoltageFailures,
            overvoltageWarnings =
                this->overvoltageWarnings + other.overvoltageWarnings,
            overloadFailures = this->overloadFailures + other.overloadFailures,
            overloadWarnings = this->overloadWarnings + other.overloadWarnings,
            phaseImbalanceFailures =
                this->phaseImbalanceFailures + other.phaseImbalanceFailures,
            phaseImbalanceWarnings =
                this->phaseImbalanceWarnings + other.phaseImbalanceWarnings,
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
                this->overheatingFailures + other.overheatingFailures,
            minFrequency = min(this->minFrequency, other.minFrequency),
            maxFrequency = max(this->maxFrequency, other.maxFrequency),
            minCurrent = min(this->minCurrent, other.minCurrent),
            maxCurrent = max(this->maxCurrent, other.maxCurrent),
            minVoltage = min(this->minVoltage, other.minVoltage),
            maxVoltage = max(this->maxVoltage, other.maxVoltage),
            energyConsumption = this->energyConsumption + other.energyConsumption};
    }
};

struct Snapshot
{
    int32_t updateTimestamp;
    int32_t version;
    int32_t activePowerFailureStartTS;
    int32_t activePowerFailureShiftingStartTS;
    int32_t activePowerFailureEndTS;
    int32_t lastPowerFailureDuration;
    mutable SnapSlice totalPrevDaysData;
    mutable SnapSlice dailyData;
};

struct SnapshotStorage
{
    uint32_t crc16;
    union
    {
        mutable Snapshot dataset;
        mutable uint8_t binary[sizeof(dataset)];
    };
};

union SnapshotData
{
    mutable SnapshotStorage content;
    mutable uint8_t data[sizeof(SnapshotStorage)];
};

static SnapshotData snapData;

#pragma pack(0)

// static double prevDayConsumption;

bool writeDailyLogCSVHeader(fs::File &file)
{
    if (!file)
        return false;
    return file.println(CSV_SUMMARY_HEADER) != 0;
};

bool writeDailyLogCSVDataLine(fs::File &file, esphome::ESPTime time)
{
    if (!time.is_valid() || !file)
        return false;

    double totalConsumption =
        snapData.content.dataset.dailyData.energyConsumption +
        snapData.content.dataset.totalPrevDaysData.energyConsumption;

    return file.printf(
               CSV_SUMMARY_DATALINE_FORMAT,
               time.strftime(CSV_SUMMARY_DATE_FORMAT).c_str(),
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

bool writeDailyLog(esphome::ESPTime time)
{
    if (SD.cardType() == CARD_NONE)
    {
        ESP_LOGW(TAG_SNAPSHOT, "There's no SD card to write down summary data.");
        return false;
    };

    if (!sdcard::ensure_date_dir_path(
            time, sdcard::DateDirectoryMode::BY_YEAR_THEN_BY_MONTH))
    {
        ESP_LOGE(TAG_SNAPSHOT,
                 "Unable to initialize date-dependent directory tree.");
        return false;
    }

    if (!sdcard::claim())
    {
        ESP_LOGE(TAG_SETTINGS,
                 "Unable to open file. Another file is opened already.");
        return false;
    }

    std::string filename;
    filename.resize(128);
    auto sz = sdcard::date_file(
        &filename[0],
        filename.size(),
        time, sdcard::DateDirectoryMode::BY_YEAR_THEN_BY_MONTH,
        SNAPLOG_FILE);
    filename.resize(sz);

    ESP_LOGI("SD Log", "Trying to write daily log to %s", filename.c_str());

    bool need_header = !SD.exists(filename.c_str());
    fs::File file = SD.open(filename.c_str(), FILE_APPEND, true);
    if (!file)
    {
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
    if (!is_success)
    {
        ESP_LOGW(TAG_SNAPSHOT, "Unable to write down a data log record.");
        return false;
    };

    return true;
};

std::string generateTelegramBotSummary_1(const char *source_name,
                                         const char *ha_uri,
                                         const char *grafana_uri)
{

    char buffer[512];
    double totalConsumption =
        snapData.content.dataset.dailyData.energyConsumption +
        snapData.content.dataset.totalPrevDaysData.energyConsumption;
    auto str_len = snprintf(
        buffer, sizeof(buffer), TG_SUMMARY_FORMAT_PART_1, source_name,
        snapData.content.dataset.dailyData.energyConsumption, totalConsumption,
        snapData.content.dataset.dailyData.minCurrent * VOLTAGE_LEVEL / 1000,
        snapData.content.dataset.dailyData.maxCurrent * VOLTAGE_LEVEL / 1000,
        ha_uri, grafana_uri);
    std::string out(buffer);
    out.resize(str_len);
    out.shrink_to_fit();
    return out;
};

std::string generateTelegramBotSummary_2(const char *source_name,
                                         const char *ha_uri,
                                         const char *grafana_uri,
                                         int timestamp = 0)
{

    char buffer[512];
    double total_power_loss_duration = 0;
    if(timestamp != 0) {
        switch (problems[GENERIC_POWER_FAILURE])
        {
        case ProblemState::WARNING:
            total_power_loss_duration = 0;
            ESP_LOGW(TAG_SNAPSHOT, "Need more time to initialize data.");
            break;
        case ProblemState::FAILURE:
            total_power_loss_duration = timestamp - snapData.content.dataset.activePowerFailureShiftingStartTS +
                snapData.content.dataset.dailyData.powerFailuresDuration;
            ESP_LOGW(TAG_SNAPSHOT, "Active power failure detected. Total power loss duration is %.2f second(s).", 
                total_power_loss_duration);
            break;
        default:
            total_power_loss_duration = snapData.content.dataset.dailyData.powerFailuresDuration;
            break;
        }
    } else {
        total_power_loss_duration = snapData.content.dataset.dailyData.powerFailuresDuration;
    };

    auto power_loss_minutes = floor(total_power_loss_duration / 60.0);
    auto power_loss_seconds = total_power_loss_duration - (power_loss_minutes * 60.0);
    auto str_len = snprintf(
        buffer, sizeof(buffer), TG_SUMMARY_FORMAT_PART_2, source_name,
        snapData.content.dataset.dailyData.powerFailuresCount, power_loss_minutes,
        power_loss_seconds, snapData.content.dataset.dailyData.minVoltage,
        snapData.content.dataset.dailyData.maxVoltage,
        snapData.content.dataset.dailyData.minFrequency,
        snapData.content.dataset.dailyData.maxFrequency);
    std::string out(buffer);
    out.resize(str_len);
    out.shrink_to_fit();
    return out;
};

std::string generateTelegramBotSummary_3(const char *source_name,
                                         const char *ha_uri,
                                         const char *grafana_uri)
{
    char buffer[1024];
    double totalConsumption =
        snapData.content.dataset.dailyData.energyConsumption +
        snapData.content.dataset.totalPrevDaysData.energyConsumption;
    int power_loss_minutes = static_cast<int>(
        snapData.content.dataset.dailyData.powerFailuresDuration / 60);
    int power_loss_seconds =
        snapData.content.dataset.dailyData.powerFailuresDuration -
        power_loss_minutes * 60;
    auto str_len = snprintf(
        buffer, sizeof(buffer), TG_SUMMARY_FORMAT_PART_3, source_name,
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
        snapData.content.dataset.dailyData.caseIntrusionFailures);
    std::string out(buffer);
    out.resize(str_len);
    out.shrink_to_fit();
    return out;
};

void saveToSnapshot(double currentConsumption)
{
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
    if (getProblem(Problems::GENERIC_POWER_FAILURE) == ProblemState::FAILURE)
    {
        snapData.content.dataset.activePowerFailureStartTS = powerFailureStartTS;
        snapData.content.dataset.activePowerFailureEndTS = 0;
    }
    else
    {
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
    snapData.content.crc16 = esphome::crc16(snapData.content.binary, sizeof(snapData.content.binary));
};

void loadFromSnapshot(double currentConsumption)
{
    uint16_t snapCrc = esphome::crc16(snapData.content.binary, sizeof(snapData.content.binary));
    if (snapCrc != snapData.content.crc16)
    {
        ESP_LOGW(TAG_SNAPSHOT, "Invalid CRC of stored data.");
    }
    // snapData.content.dataset.dailyData.energyConsumption = currentConsumption -
    //     snapData.content.dataset.totalPrevDaysData.energyConsumption;
    dailyFailures[Problems::GENERIC_POWER_FAILURE] =
        snapData.content.dataset.dailyData.powerFailuresCount;
    dailyPowerFailureDuration =
        snapData.content.dataset.dailyData.powerFailuresDuration;
    if (getProblem(Problems::GENERIC_POWER_FAILURE) == ProblemState::FAILURE)
    {
        powerFailureStartTS = snapData.content.dataset.activePowerFailureStartTS;
        powerFailureEndTS = 0;
    }
    else
    {
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
void commitDailyData(double currentConsumption, int currentTimestamp)
{
    snapData.content.dataset.totalPrevDaysData =
        snapData.content.dataset.dailyData +
        snapData.content.dataset.totalPrevDaysData;
    if (snapData.content.dataset.totalPrevDaysData.energyConsumption <
        currentConsumption)
    {
        snapData.content.dataset.totalPrevDaysData.energyConsumption =
            currentConsumption;
    };
    if (getProblem(Problems::GENERIC_POWER_FAILURE) == ProblemState::FAILURE)
    {
        snapData.content.dataset.totalPrevDaysData.powerFailuresDuration +=
            currentTimestamp - powerFailureStartTS;
        snapData.content.dataset.activePowerFailureStartTS = currentTimestamp;
        snapData.content.dataset.activePowerFailureEndTS = 0;
        snapData.content.dataset.dailyData.powerFailuresCount = 1;
    }
    else
    {
        snapData.content.dataset.activePowerFailureStartTS = powerFailureStartTS;
        snapData.content.dataset.activePowerFailureEndTS = powerFailureEndTS;
        snapData.content.dataset.dailyData.powerFailuresCount = 0;
    };
    snapData.content.dataset.dailyData.energyConsumption = 0;
    snapData.content.dataset.dailyData.powerFailuresDuration = 0;
    snapData.content.dataset.dailyData.minVoltage = VOLTAGE_LEVEL;
    snapData.content.dataset.dailyData.undervoltageFailures = 0;
    snapData.content.dataset.dailyData.undervoltageWarnings = 0;
    snapData.content.dataset.dailyData.maxVoltage = VOLTAGE_LEVEL;
    snapData.content.dataset.dailyData.overvoltageFailures = 0;
    snapData.content.dataset.dailyData.overvoltageWarnings = 0;
    snapData.content.dataset.dailyData.minCurrent = SUPPORTED_LOAD_LEVEL;
    snapData.content.dataset.dailyData.maxCurrent = SUPPORTED_LOAD_LEVEL;
    snapData.content.dataset.dailyData.overloadFailures = 0;
    snapData.content.dataset.dailyData.overloadWarnings = 0;
    snapData.content.dataset.dailyData.phaseImbalanceFailures = 0;
    snapData.content.dataset.dailyData.phaseImbalanceWarnings = 0;
    snapData.content.dataset.dailyData.minFrequency = FREQUENCY;
    snapData.content.dataset.dailyData.maxFrequency = FREQUENCY;
    snapData.content.dataset.dailyData.frequencyFailures = 0;
    snapData.content.dataset.dailyData.frequencyWarnings = 0;
    snapData.content.dataset.dailyData.breakerFailures = 0;
    snapData.content.dataset.dailyData.powerMeterFailures = 0;
    snapData.content.dataset.dailyData.caseIntrusionFailures = 0;
    snapData.content.dataset.dailyData.overheatingWarnings = 0;
    snapData.content.dataset.dailyData.overheatingFailures = 0;

    snapData.content.crc16 = esphome::crc16(snapData.content.binary, sizeof(snapData.content.binary));
};

void clearSnapshotData(time_t timestamp = 0)
{
    snapData.content.dataset.totalPrevDaysData.breakerFailures = 0;
    snapData.content.dataset.totalPrevDaysData.caseIntrusionFailures = 0;
    snapData.content.dataset.totalPrevDaysData.energyConsumption = 0;
    snapData.content.dataset.totalPrevDaysData.frequencyFailures = 0;
    snapData.content.dataset.totalPrevDaysData.frequencyWarnings = 0;
    snapData.content.dataset.totalPrevDaysData.maxCurrent = SUPPORTED_LOAD_LEVEL;
    snapData.content.dataset.totalPrevDaysData.minCurrent = SUPPORTED_LOAD_LEVEL;
    snapData.content.dataset.totalPrevDaysData.maxFrequency = FREQUENCY;
    snapData.content.dataset.totalPrevDaysData.minFrequency = FREQUENCY;
    snapData.content.dataset.totalPrevDaysData.maxVoltage = VOLTAGE_LEVEL;
    snapData.content.dataset.totalPrevDaysData.minVoltage = VOLTAGE_LEVEL;
    snapData.content.dataset.totalPrevDaysData.overheatingFailures = 0;
    snapData.content.dataset.totalPrevDaysData.overheatingWarnings = 0;
    snapData.content.dataset.totalPrevDaysData.overloadFailures = 0;
    snapData.content.dataset.totalPrevDaysData.overloadWarnings = 0;
    snapData.content.dataset.totalPrevDaysData.overvoltageFailures = 0;
    snapData.content.dataset.totalPrevDaysData.overvoltageWarnings = 0;
    snapData.content.dataset.totalPrevDaysData.undervoltageFailures = 0;
    snapData.content.dataset.totalPrevDaysData.undervoltageWarnings = 0;
    snapData.content.dataset.totalPrevDaysData.phaseImbalanceFailures = 0;
    snapData.content.dataset.totalPrevDaysData.phaseImbalanceWarnings = 0;
    snapData.content.dataset.totalPrevDaysData.powerFailuresCount = 0;
    snapData.content.dataset.totalPrevDaysData.powerFailuresDuration = 0;
    snapData.content.dataset.totalPrevDaysData.powerMeterFailures = 0;
    snapData.content.dataset.dailyData.breakerFailures = 0;
    snapData.content.dataset.dailyData.caseIntrusionFailures = 0;
    snapData.content.dataset.dailyData.energyConsumption = 0;
    snapData.content.dataset.dailyData.frequencyFailures = 0;
    snapData.content.dataset.dailyData.frequencyWarnings = 0;
    snapData.content.dataset.dailyData.maxCurrent = SUPPORTED_LOAD_LEVEL;
    snapData.content.dataset.dailyData.minCurrent = SUPPORTED_LOAD_LEVEL;
    snapData.content.dataset.dailyData.maxFrequency = FREQUENCY;
    snapData.content.dataset.dailyData.minFrequency = FREQUENCY;
    snapData.content.dataset.dailyData.maxVoltage = VOLTAGE_LEVEL;
    snapData.content.dataset.dailyData.minVoltage = VOLTAGE_LEVEL;
    snapData.content.dataset.dailyData.overheatingFailures = 0;
    snapData.content.dataset.dailyData.overheatingWarnings = 0;
    snapData.content.dataset.dailyData.overloadFailures = 0;
    snapData.content.dataset.dailyData.overloadWarnings = 0;
    snapData.content.dataset.dailyData.overvoltageFailures = 0;
    snapData.content.dataset.dailyData.overvoltageWarnings = 0;
    snapData.content.dataset.dailyData.undervoltageFailures = 0;
    snapData.content.dataset.dailyData.undervoltageWarnings = 0;
    snapData.content.dataset.dailyData.phaseImbalanceFailures = 0;
    snapData.content.dataset.dailyData.phaseImbalanceWarnings = 0;
    snapData.content.dataset.dailyData.powerFailuresCount = 0;
    snapData.content.dataset.dailyData.powerFailuresDuration = 0;
    snapData.content.dataset.dailyData.powerMeterFailures = 0;
    snapData.content.dataset.activePowerFailureEndTS = 0;
    snapData.content.dataset.activePowerFailureShiftingStartTS = 0;
    snapData.content.dataset.activePowerFailureStartTS = 0;
    snapData.content.dataset.updateTimestamp = static_cast<int>(timestamp);
    snapData.content.dataset.version = SNAPSHOT_DATA_VERSION;
    snapData.content.dataset.lastPowerFailureDuration = 0;

    snapData.content.crc16 = esphome::crc16(snapData.content.binary, sizeof(snapData.content.binary));
}