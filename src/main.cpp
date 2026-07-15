#include <Arduino.h>

#include "Config.h"

#include "DisplayController.h"
#include "MidiController.h"
#include "TouchSensor.h"

TouchSensor* sensors[NUM_SENSORS];

MidiController midi;

DisplayController displayCtrl;

uint32_t lastStatusUpdate  = 0;
uint32_t lastBatteryUpdate = 0;

// Batteriespannung in mV (2:1-Spannungsteiler auf dem Board)
static uint32_t readBatteryMilliVolts()
{
    return analogReadMilliVolts(PIN_BAT_VOLT) * 2;
}

// ------------------------------------------------
// Setup
// ------------------------------------------------

void setup()
{
    Serial.begin(115200);

    displayCtrl.begin();

    displayCtrl.showCalibrating();

    // Sensoren anlegen und kalibrieren (dabei nicht berühren!)
    for (uint8_t i = 0; i < NUM_SENSORS; i++)
    {
        sensors[i] = new TouchSensor(touchPins[i], midiNotes[i]);

        sensors[i]->begin();

        Serial.print("Sensor ");
        Serial.print(i);
        Serial.print(" Baseline: ");
        Serial.println(sensors[i]->baseline());
    }

    midi.begin();

    displayCtrl.showPads();

    displayCtrl.showBattery(readBatteryMilliVolts());
}

// ------------------------------------------------
// Loop
// ------------------------------------------------

void loop()
{
    for (uint8_t i = 0; i < NUM_SENSORS; i++)
    {
        sensors[i]->update();

        if (sensors[i]->pressedEvent())
        {
            midi.noteOn(sensors[i]->note(), sensors[i]->velocity());

            displayCtrl.drawPad(i, true);
        }

        if (sensors[i]->releasedEvent())
        {
            midi.noteOff(sensors[i]->note());

            displayCtrl.drawPad(i, false);
        }
    }

    midi.update();

    // Statuszeile höchstens alle 500 ms prüfen
    if (millis() - lastStatusUpdate > 500)
    {
        lastStatusUpdate = millis();

        displayCtrl.showStatus(midi.bleConnected(), midi.wifiConnected(), midi.rtpReady());
    }

    // Batterieanzeige in größeren Abständen aktualisieren
    if (millis() - lastBatteryUpdate > BATTERY_UPDATE_MS)
    {
        lastBatteryUpdate = millis();

        displayCtrl.showBattery(readBatteryMilliVolts());
    }

    delay(5);
}
