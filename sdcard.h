#pragma once

#include <string>
#include <SD.h>
#include <FS.h>
#include <vfs_api.h>
#include <esphome/core/time.h>
#include "csv_strings.h"

#define SCK = 18
#define MOSI = 19
#define MISO = 23
#define SS = 5

#define MAX_FILE_SIZE 16777216;
#define LOG_PATH "/events"
#define LOG_ARCHIVE LOG_PATH "/archive"
#define LOG_FILENAME LOG_PATH "/eventlog.csv"
#define LOG_ROTATE_FILENAME LOG_ARCHIVE "/%Y%m%d.log"

namespace sdcard
{
  static bool is_file_open{false};

  bool claim()
  {
    if (is_file_open)
      return false;

    is_file_open = true;
    return true;
  };

  bool free()
  {
    if (!is_file_open)
      return false;

    return true;
  };

  bool can_claim()
  {
    return !is_file_open && (SD.cardType() != CARD_NONE);
  };

  enum DateDirectoryMode
  {
    PLAIN = 0,
    BY_YEAR = 1,
    BY_MONTH = 2,
    BY_DAY = 3,
    BY_YEAR_THEN_BY_MONTH = 4,
    BY_YEAR_THEN_BY_MONTH_THEN_BY_DAY = 5,
    BY_MONTH_THEN_BY_DAY = 6
  };

#define PLAIN_TEMPLATE_DIR "/%Y-%m-%d" // /2024-02-29/*
#define BY_YEAR_TEMPLATE_DIR "/%Y"     // /2024/*
#define BY_MONTH_TEMPLATE_DIR "/%Y-%m" // /2024-02/*
#define BY_DAY_TEMPLATE_DIR PLAIN_TEMPLATE_DIR
#define BY_YEAR_THEN_BY_MONTH_TEMPLATE_DIR "/%Y/%m"                // /2024/02/*
#define BY_YEAR_THEN_BY_MONTH_THEN_BY_DAY_TEMPLATE_DIR "/%Y/%m/%d" // /2024/02/29/*
#define BY_MONTH_THEN_BY_DAY_TEMPLATE_DIR "/%Y-%m/%d"              // /2024-02/29/*

  const char *EMPTY_STRING = "";

  const char *generate_date_dir_path(esphome::ESPTime time, DateDirectoryMode mode = DateDirectoryMode::PLAIN,
                                     const char *filename = EMPTY_STRING)
  {
    if (!time.is_valid())
      return EMPTY_STRING;
    std::string dirname;
    switch (mode)
    {
    case DateDirectoryMode::PLAIN:
      dirname = time.strftime(PLAIN_TEMPLATE_DIR);
      break;
    case DateDirectoryMode::BY_YEAR:
      dirname = time.strftime(BY_YEAR_TEMPLATE_DIR);
      break;
    case DateDirectoryMode::BY_YEAR_THEN_BY_MONTH:
      dirname = time.strftime(BY_YEAR_THEN_BY_MONTH_TEMPLATE_DIR);
      break;
    case DateDirectoryMode::BY_YEAR_THEN_BY_MONTH_THEN_BY_DAY:
      dirname = time.strftime(BY_YEAR_THEN_BY_MONTH_THEN_BY_DAY_TEMPLATE_DIR);
      break;
    case DateDirectoryMode::BY_MONTH:
      dirname = time.strftime(BY_MONTH_TEMPLATE_DIR);
      break;
    case DateDirectoryMode::BY_MONTH_THEN_BY_DAY:
      dirname = time.strftime(BY_MONTH_THEN_BY_DAY_TEMPLATE_DIR);
      break;
    case DateDirectoryMode::BY_DAY:
      dirname = time.strftime(BY_DAY_TEMPLATE_DIR);
      break;
    default:
      dirname = "/";
      break;
    }

    if (filename != EMPTY_STRING)
    {
      dirname += "/";
      dirname += filename;
    };

    return dirname.c_str();
  };

  bool ensure_date_dir_path(esphome::ESPTime time, DateDirectoryMode mode = DateDirectoryMode::PLAIN)
  {
    auto dirname = generate_date_dir_path(time, mode);
    if (dirname == EMPTY_STRING)
      return false;
    if (SD.exists(dirname))
      return true;
    return SD.mkdir(dirname) == 1;
  };

  bool writeLogfile(esphome::ESPTime time, const char *eventType, const char *category, const char *message)
  {

    if (!time.is_valid())
      return false;

    if (!SD.exists(LOG_PATH))
    {
      if (!SD.mkdir(LOG_PATH))
      {
        ESP_LOGE("SD", "Unable to create log directory: %s", LOG_PATH);
        return false;
      };
      if (!SD.mkdir(LOG_ARCHIVE))
      {
        ESP_LOGE("SD", "Unable to create log archive directory: %s", LOG_ARCHIVE);
        return false;
      };
    }

    if (!claim())
      return false;

    bool needHeader = !SD.exists(LOG_FILENAME);

    auto logfile = SD.open(LOG_FILENAME, FILE_APPEND, true);

    if (!logfile)
    {
      ESP_LOGE("SD", "Unable to open or create event log.");
      free();
      return false;
    };

    bool needRotate = (logfile.size() - 1024) > MAX_FILE_SIZE;
    if (needRotate)
    {
      logfile.close();
      if (!SD.rename(LOG_FILENAME, time.strftime(LOG_ROTATE_FILENAME).c_str()))
      {
        ESP_LOGE("SD", "Unable to rotate logfile.");
        free();
        return false;
      };
      logfile = SD.open(LOG_FILENAME, FILE_APPEND, true);
      if (!logfile)
      {
        ESP_LOGE("SD", "Unable to open or create event log.");
        free();
        return false;
      };
      needHeader = true;
    };

    if (needHeader)
      logfile.println(CSV_EVENTLOG_HEADER);
    logfile.printf(CSV_EVENTLOG_DATALINE_FORMAT,
                   time.strftime(CSV_EVENTLOG_DATE_FORMAT), eventType, category, message);
    logfile.close();
    return free();
  };

  bool clearDirectory(const char *path)
  {

    if (!claim())
    {
      ESP_LOGW("SD", "Unable to remove files in directory %s. SD card is in use.", path);
      return false;
    }

    std::function<bool(const char *)> clear_lambda = [=](const char *path_to_clear)
    {
      auto dir = SD.open(path_to_clear);
      if (!dir.isDirectory())
      {
        ESP_LOGW("SD", "Provided path to clear (%s) is not a directory.", path);
        return false;
      };
      auto file = dir.openNextFile();
      while (file)
      {
        auto path_to_delete = file.path();

        if (file.isDirectory())
        {
          if (!clear_lambda(path_to_delete))
            return false;
        }
        else
        {
          file.close();
          if (!SD.remove(path_to_delete))
          {
            ESP_LOGE("SD", "Unable to delete file %s.", path_to_delete);
            return false;
          }
        }
        file = dir.openNextFile();
      };

      return false;
    };

    if (!clear_lambda(path))
    {
      free();
      return false;
    };

    return free();
  };

  bool deleteArchiveLogs()
  {
    return clearDirectory(LOG_ARCHIVE);
  };

}; // namespace sdcard
