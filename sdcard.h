#pragma once

#include "csv_strings.h"
#include <FS.h>
#include <SD.h>
#include <esphome/core/time.h>
#include <string>
#include <vfs_api.h>

#define SCK = 18
#define MOSI = 19
#define MISO = 23
#define SS = 5

#define MAX_FILE_SIZE 16777216
#define LOG_PATH "/events"
#define LOG_ARCHIVE LOG_PATH "/archive"
#define LOG_FILENAME LOG_PATH "/eventlog.csv"
#define LOG_ROTATE_FILENAME LOG_ARCHIVE "/%Y%m%d.log"

namespace sdcard
{
  static bool is_file_open = false;

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

    is_file_open = false;

    return true;
  };

  bool can_claim() { return !is_file_open && (SD.cardType() != CARD_NONE); };

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

#define PATH_DELIMITER "/"

#define PLAIN_TEMPLATE_DIR PATH_DELIMITER "%Y-%m-%d" // /2024-02-29/*
#define BY_YEAR_TEMPLATE_DIR PATH_DELIMITER "%Y"     // /2024/*
#define BY_MONTH_TEMPLATE_DIR PATH_DELIMITER "%Y-%m" // /2024-02/*
#define BY_DAY_TEMPLATE_DIR PLAIN_TEMPLATE_DIR
#define BY_YEAR_THEN_BY_MONTH_TEMPLATE_DIR PATH_DELIMITER "%Y" PATH_DELIMITER "%m" // /2024/02/*
#define BY_YEAR_THEN_BY_MONTH_THEN_BY_DAY_TEMPLATE_DIR \
  PATH_DELIMITER "%Y" PATH_DELIMITER "%m" PATH_DELIMITER "%d"                        // /2024/02/29/*
#define BY_MONTH_THEN_BY_DAY_TEMPLATE_DIR PATH_DELIMITER "%Y-%m" PATH_DELIMITER "%d" // /2024-02/29/*

  const char *EMPTY_STRING = "";

  size_t date_file(char *buffer, size_t size, esphome::ESPTime time,
                   DateDirectoryMode mode = DateDirectoryMode::PLAIN,
                   const char *filename = EMPTY_STRING)
  {
    if (!time.is_valid())
      return -1;

    switch (mode)
    {
    case DateDirectoryMode::PLAIN:
      return snprintf(buffer, size, "%s/%s", time.strftime(PLAIN_TEMPLATE_DIR).c_str(), filename);
    case DateDirectoryMode::BY_YEAR:
      return snprintf(buffer, size, "%s/%s", time.strftime(BY_YEAR_TEMPLATE_DIR).c_str(), filename);
    case DateDirectoryMode::BY_YEAR_THEN_BY_MONTH:
      return snprintf(buffer, size, "%s/%s", time.strftime(BY_YEAR_THEN_BY_MONTH_TEMPLATE_DIR).c_str(), filename);
    case DateDirectoryMode::BY_YEAR_THEN_BY_MONTH_THEN_BY_DAY:
      return snprintf(buffer, size, "%s/%s", time.strftime(BY_YEAR_THEN_BY_MONTH_THEN_BY_DAY_TEMPLATE_DIR).c_str(), filename);
    case DateDirectoryMode::BY_MONTH:
      return snprintf(buffer, size, "%s/%s", time.strftime(BY_MONTH_TEMPLATE_DIR).c_str(), filename);
    case DateDirectoryMode::BY_MONTH_THEN_BY_DAY:
      return snprintf(buffer, size, "%s/%s", time.strftime(BY_MONTH_THEN_BY_DAY_TEMPLATE_DIR).c_str(), filename);
    case DateDirectoryMode::BY_DAY:
      return snprintf(buffer, size, "%s/%s", time.strftime(BY_DAY_TEMPLATE_DIR).c_str(), filename);
    default:
      return snprintf(buffer, size, "/%s", filename);
    }
  }

  size_t date_path(char *buffer, size_t size, esphome::ESPTime time,
                   DateDirectoryMode mode = DateDirectoryMode::PLAIN)
  {
    if (!time.is_valid())
      return -1;

    switch (mode)
    {
    case DateDirectoryMode::PLAIN:
      return time.strftime(buffer, size, PLAIN_TEMPLATE_DIR);
    case DateDirectoryMode::BY_YEAR:
      return time.strftime(buffer, size, BY_YEAR_TEMPLATE_DIR);
    case DateDirectoryMode::BY_YEAR_THEN_BY_MONTH:
      return time.strftime(buffer, size, BY_YEAR_THEN_BY_MONTH_TEMPLATE_DIR);
    case DateDirectoryMode::BY_YEAR_THEN_BY_MONTH_THEN_BY_DAY:
      return time.strftime(buffer, size,
                           BY_YEAR_THEN_BY_MONTH_THEN_BY_DAY_TEMPLATE_DIR);
    case DateDirectoryMode::BY_MONTH:
      return time.strftime(buffer, size, BY_MONTH_TEMPLATE_DIR);
    case DateDirectoryMode::BY_MONTH_THEN_BY_DAY:
      return time.strftime(buffer, size, BY_MONTH_THEN_BY_DAY_TEMPLATE_DIR);
    case DateDirectoryMode::BY_DAY:
      return time.strftime(buffer, size, BY_DAY_TEMPLATE_DIR);
    default:
      return snprintf(buffer, size, "/");
    }
  }

  bool ensure_date_dir_path(esphome::ESPTime time,
                            DateDirectoryMode mode = DateDirectoryMode::PLAIN)
  {

    std::string dirname;
    dirname.resize(128);
    auto sz = date_path(&dirname[0], dirname.size(), time, mode);
    dirname.resize(sz);

    ESP_LOGI("SD", "Ensure date-dependent path: %s", dirname.c_str());
    // if (dirname.empty())
    //   return false;
    if (SD.exists(dirname.c_str()))
    {
      ESP_LOGI("SD", "Directory %s exists.", dirname.c_str());
      return true;
    }
    else
    {
      ESP_LOGI("SD", "Directory %s is not exists, trying to create it.", dirname.c_str());
      char path[255] = {'\0'}; // Max allowed path in FAT16.
      size_t path_size = dirname.size();
      path[0] = dirname[0];
      if (path_size >= 254)
      {
        ESP_LOGE("SD", "Date-dependent path tree is too long.");
        return false;
      }
      if (path[0] != '/')
      {
        ESP_LOGE("SD", "Malformed date-dependent path: %s", dirname.c_str());
        return false;
      }
      for (int i = 1; i < path_size; i++)
      {
        if ((dirname[i] == '/'))
        {
          ESP_LOGI("SD", "Creating path element: %s", &path[0]);
          if (!SD.mkdir(&path[0]))
          {
            ESP_LOGE("SD", "Unable to create date-dependent dir tree element: %s", &path[0]);
            return false;
          };
        };
        path[i] = dirname[i];
        if (i == (path_size - 1))
        {
          path[i + 1] = '\0';
          ESP_LOGI("SD", "Creating the last path element: %s", &path[0]);
          if (!SD.mkdir(&path[0]))
          {
            ESP_LOGE("SD", "Unable to create date-dependent dir tree element: %s", &path[0]);
            return false;
          };
          return true;
        }
      };
    };
    return false;
  };

  bool writeLogfile(esphome::ESPTime time, const char *eventType,
                    const char *category, const char *message)
  {

    if (!time.is_valid())
      return false;

    if (!SD.exists(LOG_PATH))
    {
      ESP_LOGW("SD", "Log path not found. Trying to create log directory: %s.",
               LOG_PATH);
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

    if (needHeader)
    {
      ESP_LOGI("SD Log",
               "Log file %s not found. Trying to create and empty CSV log.",
               LOG_FILENAME);
    }

    auto logfile = SD.open(LOG_FILENAME, FILE_APPEND, true);

    if (!logfile)
    {
      ESP_LOGE("SD", "Unable to open or create event log.");
      free();
      return false;
    };

    long filesize = 0;
    filesize = logfile.size();
    ESP_LOGI("SD Log", "Log file size is %.2f Mb", filesize / 1024.0 / 1024.0);

    bool needRotate = (filesize > 0) && (filesize > MAX_FILE_SIZE);
    if (needRotate)
    {
      const char *filename = time.strftime(LOG_ROTATE_FILENAME).c_str();
      ESP_LOGW("SD Log", "Need to rotate log: %s >>> %s.", LOG_FILENAME,
               filename);
      logfile.close();
      if (!SD.rename(LOG_FILENAME, filename))
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
                   time.strftime(CSV_EVENTLOG_DATE_FORMAT).c_str(), eventType, category,
                   message);
    ESP_LOGI("SD Log", "Log record has been added. Log file size: %.2f Kb",
             logfile.size() / 1024.0);
    logfile.close();
    return free();
  };

  bool clearDirectory(const char *path)
  {

    if (!claim())
    {
      ESP_LOGW("SD", "Unable to remove files in directory %s. SD card is in use.",
               path);
      return false;
    }

    std::function<bool(const char *)> clear_lambda =
        [=](const char *path_to_clear)
    {
      auto dir = SD.open(path_to_clear);
      if (!dir.isDirectory())
      {
        ESP_LOGW("SD", "Provided path to clear (%s) is not a directory.",
                 path);
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

  bool deleteArchiveLogs() { return clearDirectory(LOG_ARCHIVE); };

}; // namespace sdcard
