#pragma once

#define USE_SD_CARD_NO

#include <esphome/core/helpers.h>

#ifdef USE_SD_CARD

#include <SD.h>
#include <FS.h>
#include "sdcard.h"

#endif


#define VOLTAGE_LEVEL 230
#define FREQUENCY 50
#define SUPPORTED_LOAD_LEVEL 32
#define OVERHEATING_WARNING_TEMPERATURE 50
#define OVERHEATING_FAILURE_TEMPERATURE 120

#define TAG_SETTINGS "Settings"

#define SETTINGS_FILE "/settings.dat"

namespace settings
{


    struct NodeSettings {
        float undervoltageWarningLevel;
        float undervoltageFailureLevel;
        float overvoltageWarningLevel;
        float overvoltageFailureLevel; 
        float frequencyShiftWarningLevel;
        float frequencyShiftFailureLevel;
        float phaseShiftWarningLevel;
        float phaseShiftFailureLevel;
        float overloadWarningLevel;
        float overloadFailureLevel;
        bool publishSummary;
        int monthlyReportDay;
        uint8_t gatewayNodePowerPolicy;
    };

    union NodeSettingsBinary
    {
        struct
        {
            union {
                NodeSettings settings;
                uint8_t data[sizeof(settings)];
            };
            uint16_t hash;
        }  content;
        uint8_t data[sizeof(content)];
    };

    static NodeSettingsBinary settingsData{};


//    NodeSettings settings() { return &settingsData.content; };

#ifdef USE_SD_CARD
    /* Writes settings to SD card */
    bool writeSettings()
    {
        if(SD.cardType() == CARD_NONE)
        {
            ESP_LOGW(TAG_SETTINGS, "There's no SD card to save settings.");
            return true;
        }

        if(!sdcard::claim()) {
            ESP_LOGE(TAG_SETTINGS, "Unable to open file. Another file is opened already.");
            return false;
        }

        auto settingsFile = SD.open(SETTINGS_FILE, FILE_WRITE, true);
        if(!settingsFile) {
            ESP_LOGE(TAG_SETTINGS, "Unable to open settings file.");
            sdcard::free();
            return false;
        }

        settingsData.content.hash = esphome::crc16(settingsData.content.data, sizeof(settingsData.content.data));

        if(settingsFile.write(settingsData.data, sizeof(settingsData.data)) == 0) {
            ESP_LOGE(TAG_SETTINGS, "Unable to save settings file.");
            settingsFile.close();
            sdcard::free();
            return false;
        }

        settingsFile.close();
        sdcard::free();

        return true;
    }    
#else

    unsigned char *storage{nullptr};

    bool writeSettings() {
        if(storage == nullptr)
        {
            ESP_LOGE(TAG_SETTINGS, "Settings storage variable is not defined. Please assert uint8_t array global to 'storage' pointer.");
            return false;
        }

        settingsData.content.hash = esphome::crc16(settingsData.content.data, sizeof(settingsData.content.data));
        ESP_LOGD(TAG_SETTINGS, "Settings CRC is %0x", settingsData.content.hash);
        for (size_t i = 0; i < sizeof(settingsData.data); i++)
        {
            storage[i] = settingsData.data[i];
        }

        ESP_LOGI(TAG_SETTINGS, "Settings has been saved to internal memory.");
        return true;        
    }
#endif

    /* Reset settings to default and optionally saves it to sd card. */
    bool resetSettings(bool saveFile = false){

        settingsData.content.settings.undervoltageWarningLevel = 0.85 * VOLTAGE_LEVEL;
        settingsData.content.settings.undervoltageFailureLevel = 0.60 * VOLTAGE_LEVEL;
        settingsData.content.settings.overvoltageWarningLevel = 1.05 * VOLTAGE_LEVEL;
        settingsData.content.settings.overvoltageFailureLevel = 1.15 * VOLTAGE_LEVEL;
        settingsData.content.settings.frequencyShiftFailureLevel = 1.5;
        settingsData.content.settings.frequencyShiftWarningLevel = 0.2;
        settingsData.content.settings.phaseShiftWarningLevel = 5.0;
        settingsData.content.settings.phaseShiftFailureLevel = 15.0;
        settingsData.content.settings.overloadWarningLevel = 0.90 * SUPPORTED_LOAD_LEVEL;
        settingsData.content.settings.overloadFailureLevel = 1.05 * SUPPORTED_LOAD_LEVEL;
        settingsData.content.settings.gatewayNodePowerPolicy = 0;
        settingsData.content.settings.publishSummary = true;
        settingsData.content.settings.monthlyReportDay = 1;

        if(saveFile)
            return writeSettings();
        else 
            return true;
    }

#ifdef USE_SD_CARD
    /* Reads settings from sd card */
    bool readSettings()
    {
        if(SD.cardType() == CARD_NONE)
        {
            ESP_LOGW(TAG_SETTINGS, "There's no SD card to load settings.");
            return true;
        }

        if(!SD.exists(SETTINGS_FILE))
        {
            ESP_LOGW(TAG_SETTINGS, "Settings file not found. Creating the defaults one.");
            return resetSettings(true);
        }

        if(!sdcard::claim()) {
            ESP_LOGE(TAG_SETTINGS, "Unable to open file. Another file is opened already.");
            return false;
        }

        auto settingsFile = SD.open(SETTINGS_FILE, FILE_READ);
        if(!settingsFile) {
            ESP_LOGE(TAG_SETTINGS, "Unable to open settings file.");
            sdcard::free();
            return false;
        }

        NodeSettingsBinary loaded{};

        if(settingsFile.read(loaded.data, sizeof(loaded.data)) == -1)
        {
            ESP_LOGE(TAG_SETTINGS, "Unable to read setings file.");
            settingsFile.close();
            sdcard::free();
            return false;
        }

        settingsFile.close();
        sdcard::free();
        
        uint16_t loaded_crc = esphome::crc16(loaded.content.data, sizeof(loaded.content.data));
        if(loaded_crc != loaded.content.hash)
        {
            ESP_LOGE(TAG_SETTINGS, "Invalid checksum. Settings file is corrupted. Using default values.");
            return resetSettings(true);
        }

        memcpy(settingsData.data, loaded.data, sizeof(loaded.data));

        return true;
    }
#else
    bool readSettings() {
        if(storage == nullptr)
        {
            ESP_LOGE(TAG_SETTINGS, "Settings storage variable is not defined. Please assert uint8_t array global to 'storage' pointer.");
            return false;
        }

        NodeSettingsBinary loaded{};

        for (size_t i = 0; i < sizeof(settingsData.data); i++)
        {
            loaded.data[i] = storage[i];
        }

        uint16_t loaded_crc = esphome::crc16(loaded.content.data, sizeof(loaded.content.data));
        if(loaded_crc != loaded.content.hash)
        {
            ESP_LOGE(TAG_SETTINGS, "Invalid checksum. Settings file is corrupted. Using default values.");
            return resetSettings(true);
        }

        memcpy(settingsData.data, loaded.data, sizeof(loaded.data));

        ESP_LOGI(TAG_SETTINGS, "Settings has been read from internal memory.");
        return true;
    }

#endif
} // namespace settings


