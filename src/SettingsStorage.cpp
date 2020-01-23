#include "SettingsStorage.h"

#include <StreamUtils.h>

#ifdef ESP8266
#include <FS.h>
#elif ESP32
#include <FS.h>
#include <SPIFFS.h>
#else
#error Platform not supported.
#endif


void SettingsStorage::setup() {
    loadSettings();
}

int8_t SettingsStorage::saveSettings() {
    return saveSettings(absolute_file_path.c_str(), getDocumentRoot());
}

int8_t SettingsStorage::saveSettings(const char *file_name, JsonVariant config) {
    if (!SPIFFS.begin())
        return -1;

    File file = SPIFFS.open(file_name, "w");
    if (!file) {
        SPIFFS.end();
        return -1;
    } else {
        WriteBufferingStream buffered_writer(file, 64);
        serializeJson(config, buffered_writer);
        buffered_writer.flush();
    }
    file.close();

    SPIFFS.end();
    return 0;
}

int8_t SettingsStorage::loadSettings() {
    return loadSettings(absolute_file_path.c_str());
}

int8_t SettingsStorage::loadSettings(const char *file_name) {
    Serial.println("SettingsStorage::loadSettings");

    if (!SPIFFS.begin()) {
        Serial.println("SettingsStorage::loadSettings: failed to open spiffs");
        return -1;
    }

    File file = SPIFFS.open(file_name, "r");
    if (!file) {
        Serial.printf("SettingsStorage::loadSettings: failed to open file %s\n", file_name);
        SPIFFS.end();
        return -1;
    } else {
        ReadBufferingStream buffered_reader(file, 64);
        DeserializationError e = deserializeJson(document, buffered_reader);
        if (e) {
            Serial.printf("SettingsStorage::loadSettings: failed to load file due to %s\n", e.c_str());
            document.clear();
            file.close();
            SPIFFS.end();
            return -1;
        }
    }
    file.close();
    SPIFFS.end();
    Serial.println("SettingsStorage::loadSettings: loaded");
    return 0;
}

const JsonVariant SettingsStorage::getDocumentRoot() const {
    JsonVariant ref = const_cast<SettingsStorage *>(this)->document.as<JsonVariant>();
    return *static_cast<const JsonVariant *>(&(ref));
}

JsonVariant SettingsStorage::getDocumentRoot() {
    return document.as<JsonVariant>();
}

JsonDocument &SettingsStorage::getDocument() {
    return document;
}

void SettingsStorage::print() const {
    Serial.println();
    serializeJson(document, Serial);
    Serial.println();
}

void SettingsStorage::printMemoryUsage() const {
    Serial.printf("SettingsStorage::printMemoryUsage: consumed %d of %d\n", document.memoryUsage(),
                  document.capacity());
}

SettingsStorage::SettingsStorage(const String &file_path) : absolute_file_path(file_path) {
}


