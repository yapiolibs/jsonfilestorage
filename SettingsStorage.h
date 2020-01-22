#pragma once

#include <ArduinoJson.h>

struct SettingsStorage
{

    explicit SettingsStorage(const String &file_path = "/settings.json");

    void setup();

    int8_t loadSettings();

    int8_t saveSettings();


    JsonVariant getDocumentRoot() ;

    JsonDocument &getDocument();

    void print() const;

    void printMemoryUsage() const;

protected:

    virtual int8_t loadSettings(const char *file_name);

    int8_t saveSettings(const char *file_name, JsonVariant config);

    String absolute_file_path;

    DynamicJsonDocument document{4500};

    const JsonVariant getDocumentRoot() const;

};

