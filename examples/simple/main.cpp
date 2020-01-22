#include <HardwareSerial.h>
#include <WiFiManager.h>
#include <main/KeyEventHandler.h>
#include <main/StatusBar.h>
#include "display/Display.h"
#include "main/OperatingMode.h"
#include "expansion_board/ExpansionBoard.h"
#include "relay_board/settings/KeyRelaySettingsHelper.h"
#include "relay_board/settings/KeyRelaySettingsStorage.h"
#include "wifi/WifiConfigOrFallbackAccesspointManager.h"
#include "main/StatusAndConfigurationWebService.h"

struct Resources
{
    struct EarlyInitializer
    {
        EarlyInitializer()
        {
            Serial.begin(230400);
            while (!Serial) delay(10);
            Serial.printf("\n\n\n");

            wifi_status_led_uninstall();
            pinMode(LED_BUILTIN, OUTPUT);
            digitalWrite(LED_BUILTIN, HIGH);
        }
    } early_init;

    OperatingMode operating_mode;
    Keyboard keyboard;
    Display display;
    StatusBar status_bar{operating_mode, display};
    KeyRelaySettingsStorage settings;
    ExpansionBoard io_expander;
    RelaysBoard relays_board{io_expander};
    StandbyOfficer standby_officer{30};
    KeyRelaySettings relays_actions{};
    KeyEventHandler event_handler{operating_mode, standby_officer, display, relays_actions, relays_board};
    StatusAndConfigurationWebService web_service{settings, relays_actions};

    void setup()
    {
        // --- load settings
        [&]()
        {
            // --- load last state
            settings.setup();
            settings.writeKeyRelayActions(relays_actions);

            // --- informational output
            KeyEventsRelaysActionHelper<KeyRelaySettings>(relays_actions).print();

            settings.print();
            settings.printMemoryUsage();
            //settings.resetDocument();
        }();

        // --- restore physical state
        [&]()
        {
            // --- restore last stored state
            io_expander.setup(relays_actions.relay_flags);
            relays_board.setup();
        }();

        // --- ui periphery
        [&]()
        {
            display.setup();
            keyboard.setup(&event_handler);
        }();

        // -- power save
        [&]()
        {
            // --- auto standby
            standby_officer.enable();
        }();
    }

    void process()
    {
        if (operating_mode == OperatingMode::Mode::Normal)
            processNormalMode();
        else if (operating_mode == OperatingMode::Mode::SwitchToWifi)
            processSwitchToWifi();
        else if (operating_mode == OperatingMode::Mode::Wifi)
            processWifi();
    }

    void processNormalMode()
    {
        standby_officer.process([]()
                                {});
        if (keyboard.process())
        {
            status_bar.update();
        }
    }

    void processSwitchToWifi()
    {
        WiFi.mode(WIFI_AP);
        status_bar.update();
        WifiConfigOrFallbackAccesspointManager foo(display);

        status_bar.update();
        web_service.setup();
        operating_mode.setMode(OperatingMode::Mode::Wifi);
    }

    void processWifi()
    {
        if (web_service.handleClient())
        {
            status_bar.update();
        }
        processNormalMode();
    }
} r;

void setup()
{ r.setup(); }

void loop()
{ r.process(); }
