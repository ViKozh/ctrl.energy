#pragma once

#include "settings.h"
#include <esphome/core/helpers.h>
#include <esphome/core/time.h>
#include <map>
#include "tg_bot_strings.h"

/* Global constants */
static const int PROBLEMS_COUNT = 12;

static const esphome::ESPTime NONE_TIME = esphome::ESPTime{};

/* Problems enumeration */
enum Problems
{
  GENERIC_POWER_FAILURE = 0,
  UNDERVOLTAGE = 1,
  OVERVOLTAGE = 2,
  OVERLOAD = 3,
  PHASE_SHIFT = 4,
  FREQUENCY_SHIFT = 5,
  BREAKER = 6,
  POWER_METER = 7,
  INTRUSION = 8,
  BATTERY = 9,
  OVERHEAT = 10,
  AC_LINE = 11
};

/* Problems state */
enum ProblemState
{
  NONE = 0,
  WARNING = 1,
  FAILURE = 2
};

static esphome::CallbackManager<void(Problems)> problem_failure_callback;
static esphome::CallbackManager<void(Problems)> problem_warning_callback;
static esphome::CallbackManager<void(Problems)> problem_restore_callback;

void add_on_failure_callback(std::function<void(Problems)> &&callback) { problem_failure_callback.add(std::move(callback)); };
void add_on_warning_callback(std::function<void(Problems)> &&callback) { problem_warning_callback.add(std::move(callback)); };
void add_on_restore_callback(std::function<void(Problems)> &&callback) { problem_restore_callback.add(std::move(callback)); };

/* State names */
static const std::map<ProblemState, const char *> STATE_NAMES{
    {ProblemState::NONE, "OK"},
    {ProblemState::WARNING, "Warning"},
    {ProblemState::FAILURE, "Failure"}};

/* Keys for summary data on problems */
static const std::map<Problems, const char *> PROBLEMS_KEYS{
    {Problems::GENERIC_POWER_FAILURE, "PowerLoss"},
    {Problems::UNDERVOLTAGE, "Undervoltage"},
    {Problems::OVERVOLTAGE, "Overvoltage"},
    {Problems::OVERLOAD, "Overload"},
    {Problems::PHASE_SHIFT, "PhaseShift"},
    {Problems::FREQUENCY_SHIFT, "Frequency_Shift"},
    {Problems::BREAKER, "CircuitBreaker"},
    {Problems::POWER_METER, "PowerMeterConnectivity"},
    {Problems::INTRUSION, "CaseIntrusion"},
    {Problems::BATTERY, "NodeUPSBattery"},
    {Problems::OVERHEAT, "Overheat"},
    {Problems::AC_LINE, "NodeACPower"}};

static const std::map<Problems, const char *> PROBLEMS_NAMES{
    {Problems::GENERIC_POWER_FAILURE, "Power Loss"},
    {Problems::UNDERVOLTAGE, "Undervoltage"},
    {Problems::OVERVOLTAGE, "Overvoltage"},
    {Problems::OVERLOAD, "Overload"},
    {Problems::PHASE_SHIFT, "Phase Shift"},
    {Problems::FREQUENCY_SHIFT, "Frequency Shift"},
    {Problems::BREAKER, "Circuit Breaker"},
    {Problems::POWER_METER, "Power Meter Connectivity"},
    {Problems::INTRUSION, "Case Intrusion"},
    {Problems::BATTERY, "Node UPS Battery"},
    {Problems::OVERHEAT, "Overheat"},
    {Problems::AC_LINE, "Node AC Power"}};

static const std::map<Problems, const char *> PROBLEMS_MEASURES{
    {Problems::GENERIC_POWER_FAILURE, ""},
    {Problems::UNDERVOLTAGE, "V"},
    {Problems::OVERVOLTAGE, "V"},
    {Problems::OVERLOAD, "A"},
    {Problems::PHASE_SHIFT, "%"},
    {Problems::FREQUENCY_SHIFT, "Hz"},
    {Problems::BREAKER, ""},
    {Problems::POWER_METER, ""},
    {Problems::INTRUSION, ""},
    {Problems::BATTERY, ""},
    {Problems::OVERHEAT, "°C"},
    {Problems::AC_LINE, ""}};

static ProblemState problems[PROBLEMS_COUNT];
static int dailyWarnings[PROBLEMS_COUNT];
static int dailyFailures[PROBLEMS_COUNT];
static double minVoltage, maxVoltage, minFrequency, maxFrequency, minCurrent, maxCurrent;
static int lastPowerFailureDuration = 0;

// Stores actual start of power failure
static int powerFailureShiftingStartTS = 0;
// Stores start of power failure event or midnight if powerfailure duration
// moves over it.
static int powerFailureStartTS = 0;
static int powerFailureEndTS = 0;
static int dailyPowerFailureDuration = 0;

static bool isActive;

/// @brief Sets the problem state and updates daily counters for it.
/// @param problem id to report
/// @param state A state of problem to set
void setProblem(Problems problem, ProblemState state = ProblemState::NONE,
                int timestamp = 0)
{
  if (!isActive)
    return;

  int idx = static_cast<int>(problem);
  auto prev_ = problems[idx];
  if (prev_ == state)
    return;

  switch (state)
  {
  case ProblemState::WARNING:
    dailyWarnings[idx] = dailyWarnings[idx] + 1;
    // if (prev_ != ProblemState::UNKNOWN)
    problem_warning_callback.call(problem);
    break;
  case ProblemState::FAILURE:
    dailyFailures[idx] = dailyFailures[idx] + 1;
    if (problem == Problems::GENERIC_POWER_FAILURE && (timestamp != 0))
    {
      powerFailureShiftingStartTS = timestamp;
      powerFailureStartTS = timestamp;
      lastPowerFailureDuration = -1;
      powerFailureEndTS = 0;
    };
    // if (prev_ != ProblemState::UNKNOWN)
    problem_failure_callback.call(problem);
    break;
  case ProblemState::NONE:
    if (problem == Problems::GENERIC_POWER_FAILURE && (timestamp != 0) &&
        powerFailureStartTS != 0)
    {
      powerFailureEndTS = timestamp;
      lastPowerFailureDuration = powerFailureEndTS - powerFailureShiftingStartTS;
      dailyPowerFailureDuration +=
          static_cast<int>((powerFailureEndTS - powerFailureStartTS));
    }
    // if (prev_ != ProblemState::UNKNOWN)
    problem_restore_callback.call(problem);
    break;
  default:
    break;
  }

  problems[static_cast<int>(problem)] = state;
};

/// @brief Gets state of specified problem
/// @param problem id to get
/// @return A state of problem
ProblemState getProblem(Problems problem)
{
  return problems[static_cast<int>(problem)];
};

/// @brief Gets state of specified problem
/// @param problem id to get
/// @return A state of problem
ProblemState getProblem(int i)
{
  return getProblem(static_cast<Problems>(i));
};

void startMonitoring()
{
  isActive = true;
}

void stopMonitoring()
{
  isActive = false;
}

void setupProblems()
{
  isActive = false;
  for (int i = 0; i < PROBLEMS_COUNT; i++)
  {
    problems[i] = ProblemState::NONE;
    dailyFailures[i] = 0;
    dailyWarnings[i] = 0;
  };
  dailyPowerFailureDuration = 0;
  minVoltage = VOLTAGE_LEVEL;
  maxVoltage = VOLTAGE_LEVEL;
  minCurrent = SUPPORTED_LOAD_LEVEL;
  maxCurrent = SUPPORTED_LOAD_LEVEL;
  minFrequency = FREQUENCY;
  maxFrequency = FREQUENCY;
};

/// @brief Resets daily counters for problems
void resetCounters()
{
  for (int i = 0; i < PROBLEMS_COUNT; i++)
  {
    dailyWarnings[i] = 0;
    dailyFailures[i] = 0;
  };
  dailyPowerFailureDuration = 0;
  minVoltage = VOLTAGE_LEVEL;
  maxVoltage = VOLTAGE_LEVEL;
  minCurrent = SUPPORTED_LOAD_LEVEL;
  maxCurrent = SUPPORTED_LOAD_LEVEL;
  minFrequency = FREQUENCY;
  maxFrequency = FREQUENCY;
};

void monitorVoltage(double phaseA, double phaseB, double phaseC)
{
  if (!isActive)
    return;

  if (getProblem(Problems::GENERIC_POWER_FAILURE) != ProblemState::NONE)
  {
    // Nothing to do when powered off.
    return;
  }

  bool is_finite = isfinite(phaseA) &&
                   isfinite(phaseB) &&
                   isfinite(phaseC) &&
                   phaseA >= 0 &&
                   phaseB >= 0 &&
                   phaseC >= 0;

  if (!is_finite)
    return;

  minVoltage = min(phaseA, min(phaseB, phaseC));
  maxVoltage = max(phaseA, max(phaseB, phaseC));
  if (maxVoltage >=
      settings::settingsData.content.settings.overvoltageFailureLevel)
  {
    setProblem(Problems::OVERVOLTAGE, ProblemState::FAILURE);
  }
  else if (maxVoltage >=
           settings::settingsData.content.settings.overloadWarningLevel)
  {
    setProblem(Problems::OVERVOLTAGE, ProblemState::WARNING);
  }
  else
  {
    setProblem(Problems::OVERVOLTAGE, ProblemState::NONE);
  }

  if (minVoltage <=
      settings::settingsData.content.settings.undervoltageFailureLevel)
  {
    setProblem(Problems::UNDERVOLTAGE, ProblemState::FAILURE);
  }
  else if (minVoltage <=
           settings::settingsData.content.settings.undervoltageWarningLevel)
  {
    setProblem(Problems::UNDERVOLTAGE, ProblemState::WARNING);
  }
  else
  {
    setProblem(Problems::UNDERVOLTAGE, ProblemState::NONE);
  }
}

void monitorPhaseShift(double value)
{
  if (!isActive)
    return;

  if (getProblem(Problems::GENERIC_POWER_FAILURE) != ProblemState::NONE)
  {
    // Nothing to do when powered off.
    return;
  }

  auto shift_abs = abs(value);
  if (shift_abs >=
      settings::settingsData.content.settings.phaseShiftFailureLevel)
  {
    setProblem(Problems::PHASE_SHIFT, ProblemState::FAILURE);
  }
  else if (shift_abs >=
           settings::settingsData.content.settings.phaseShiftWarningLevel)
  {
    setProblem(Problems::PHASE_SHIFT, ProblemState::WARNING);
  }
  else
  {
    setProblem(Problems::PHASE_SHIFT);
  };
};

void monitorCurrent(double phaseA, double phaseB, double phaseC)
{
  if (!isActive)
    return;

  if (getProblem(Problems::GENERIC_POWER_FAILURE) != ProblemState::NONE)
  {
    // Nothing to do when powered off.
    return;
  }

  bool is_finite = isfinite(phaseA) &&
                   isfinite(phaseB) &&
                   isfinite(phaseC) &&
                   phaseA >= 0 &&
                   phaseB >= 0 &&
                   phaseC >= 0;

  if (!is_finite)
    return;

  maxCurrent = max(phaseA, max(phaseB, phaseC));
  minCurrent = min(phaseA, min(phaseB, phaseC));
  auto avgCurrent = (phaseA + phaseB + phaseC) / 3;
  if (maxCurrent >=
      settings::settingsData.content.settings.overloadFailureLevel)
  {
    setProblem(Problems::OVERLOAD, ProblemState::FAILURE);
  }
  else if (maxCurrent >=
           settings::settingsData.content.settings.overloadWarningLevel)
  {
    setProblem(Problems::OVERLOAD, ProblemState::WARNING);
  }
  else
  {
    setProblem(Problems::OVERLOAD);
  }

  auto maxShift = max(abs(phaseA / avgCurrent - 1.0),
                      max(abs(phaseB / avgCurrent - 1.0), abs(phaseC / avgCurrent - 1.0)));

  monitorPhaseShift(maxShift * 100);
};

void monitorFrequencyShift(double value)
{
  if (!isActive)
    return;

  if (getProblem(Problems::GENERIC_POWER_FAILURE) != ProblemState::NONE)
  {
    // Nothing to do when powered off.
    return;
  }

  if ((!isfinite(value)) || (value < 0))
    return;

  minFrequency = min(minFrequency, value);
  maxFrequency = max(maxFrequency, value);
  auto freq_delta = abs(value - FREQUENCY);
  if (freq_delta >=
      settings::settingsData.content.settings.frequencyShiftFailureLevel)
  {
    setProblem(Problems::FREQUENCY_SHIFT, ProblemState::FAILURE);
  }
  else if (freq_delta >= settings::settingsData.content.settings
                             .frequencyShiftWarningLevel)
  {
    setProblem(Problems::FREQUENCY_SHIFT, ProblemState::WARNING);
  }
  else
  {
    setProblem(Problems::FREQUENCY_SHIFT);
  };
};

void monitorBreaker(bool isOk)
{
  if (!isActive)
    return;

  if (!isOk)
    setProblem(Problems::BREAKER, ProblemState::FAILURE);
  else
    setProblem(Problems::BREAKER, ProblemState::NONE);
};

void monitorPowerMeter(bool isOk)
{
  if (!isActive)
    return;

  if (!isOk)
    setProblem(Problems::POWER_METER, ProblemState::FAILURE);
  else
    setProblem(Problems::POWER_METER, ProblemState::NONE);
};

void monitorPowerFailure(bool isPoweredOn, int timestamp = 0)
{
  if (!isActive)
    return;

  if (!isPoweredOn)
    setProblem(Problems::GENERIC_POWER_FAILURE, ProblemState::FAILURE, timestamp);
  else
    setProblem(Problems::GENERIC_POWER_FAILURE, ProblemState::NONE, timestamp);
};

void monitorCaseIntrusion(bool isOpened)
{
  if (!isActive)
    return;

  if (isOpened)
    setProblem(Problems::INTRUSION, ProblemState::FAILURE);
  else
    setProblem(Problems::INTRUSION, ProblemState::NONE);
};

void monitorBatteryFailure(bool isOk)
{
  if (!isActive)
    return;

  if (!isOk)
    setProblem(Problems::BATTERY, ProblemState::FAILURE);
  else
    setProblem(Problems::BATTERY, ProblemState::NONE);
};

void monitorACLineFailure(bool isOk)
{
  if (!isActive)
    return;

  if (!isOk)
    setProblem(Problems::AC_LINE, ProblemState::FAILURE);
  else
    setProblem(Problems::AC_LINE, ProblemState::NONE);
}

void monitorOverheating(double temperature)
{
  if (!isActive)
    return;

  if (!isfinite(temperature))
    return;

  if (temperature >= OVERHEATING_FAILURE_TEMPERATURE)
  {
    setProblem(Problems::OVERHEAT, ProblemState::FAILURE);
  }
  else if (temperature >= OVERHEATING_WARNING_TEMPERATURE)
  {
    setProblem(Problems::OVERHEAT, ProblemState::WARNING);
  }
  else
  {
    setProblem(Problems::OVERHEAT, ProblemState::NONE);
  }
}

const char *getProblemState(Problems problem)
{
  return STATE_NAMES.at(getProblem(problem));
};

std::string generatePowerFailureMessage(const char *sourceName, ProblemState state)
{
  if (state != ProblemState::NONE)
  {
    char buffer[512];
    snprintf(buffer, sizeof(buffer), TG_POWER_FAIL,
             sourceName,
             getProblemState(Problems::BREAKER),
             getProblemState(Problems::BATTERY),
             getProblemState(Problems::AC_LINE),
             getProblemState(Problems::INTRUSION));
    return std::string(buffer);
  }
  else
  {
    int duration_minutes = static_cast<int>(lastPowerFailureDuration / 60.0);
    int duration_seconds = lastPowerFailureDuration - duration_minutes * 60;
    char buffer[256];
    snprintf(buffer, sizeof(buffer), TG_POWER_RESTORED,
             sourceName,
             duration_minutes,
             duration_seconds);
    return std::string(buffer);
  }
};

std::string generateProblemMessage(const char *sourceName, Problems problem, ProblemState state, double value = NAN)
{
  char buffer[255];
  switch (problem)
  {
  case Problems::GENERIC_POWER_FAILURE:
    return generatePowerFailureMessage(sourceName, state);
  case Problems::FREQUENCY_SHIFT:
  case Problems::OVERHEAT:
  case Problems::OVERLOAD:
  case Problems::OVERVOLTAGE:
  case Problems::PHASE_SHIFT:
  case Problems::UNDERVOLTAGE:
    switch (state)
    {
    case ProblemState::NONE:
      snprintf(buffer, sizeof(buffer), TG_RESTORE_MESSAGE_WITH_VALUE,
               PROBLEMS_NAMES.at(problem), sourceName,
               value, PROBLEMS_MEASURES.at(problem),
               dailyWarnings[problem], dailyFailures[problem]);
      break;
    case ProblemState::WARNING:
      snprintf(buffer, sizeof(buffer), TG_WARNING_MESSAGE_WITH_VALUE,
               PROBLEMS_NAMES.at(problem), sourceName,
               value, PROBLEMS_MEASURES.at(problem),
               dailyWarnings[problem]);
      break;
    case ProblemState::FAILURE:
      snprintf(buffer, sizeof(buffer), TG_FAILURE_MESSAGE_WITH_VALUE,
               PROBLEMS_NAMES.at(problem), sourceName,
               value, PROBLEMS_MEASURES.at(problem),
               dailyFailures[problem]);
      break;
    default:
      break;
    }
    return std::string(buffer);
  case Problems::AC_LINE:
  case Problems::BATTERY:
  case Problems::BREAKER:
  case Problems::POWER_METER:
  case Problems::INTRUSION:
  default:
    switch (state)
    {
    case ProblemState::NONE:
      snprintf(buffer, sizeof(buffer), TG_RESTORE_MESSAGE,
               PROBLEMS_NAMES.at(problem), sourceName,
               dailyWarnings[problem], dailyFailures[problem]);
      break;
    case ProblemState::FAILURE:
      snprintf(buffer, sizeof(buffer), TG_FAILURE_MESSAGE,
               PROBLEMS_NAMES.at(problem), sourceName,
               dailyFailures[problem]);
      break;
    case ProblemState::WARNING:
      snprintf(buffer, sizeof(buffer), TG_WARNING_MESSAGE,
               PROBLEMS_NAMES.at(problem), sourceName,
               dailyWarnings[problem]);
      break;
    default:
      snprintf(buffer, sizeof(buffer), TG_EMPTY_MESSAGE,
               sourceName);
      break;
    }
    return std::string(buffer);
  }
};
