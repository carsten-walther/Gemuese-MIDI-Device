#pragma once

#include <Arduino.h>

// ------------------------------------------------
// MIDI-Transports
// ------------------------------------------------

// BLE-MIDI: erscheint als Bluetooth-MIDI-Gerät (macOS, iOS, Windows 10+)
constexpr bool ENABLE_BLE_MIDI = true;

// RTP-MIDI (AppleMIDI) über WLAN: erscheint in Audio-MIDI-Setup (macOS)
// bzw. rtpMIDI (Windows). Braucht gültige WLAN-Zugangsdaten (unten).
constexpr bool ENABLE_WIFI_MIDI = true;

// USB-Host-MIDI: ein MIDI-Gerät wird AN den ESP32 angeschlossen (OTG).
// Achtung: belegt den USB-Port exklusiv, der Serial-Monitor über USB
// funktioniert dann nicht mehr.
constexpr bool ENABLE_USB_MIDI = false;

// Gerätename (BLE-Advertising und RTP-MIDI/mDNS)
constexpr char MIDI_DEVICE_NAME[] = "Gemuese-MIDI-Device";

// ------------------------------------------------
// WLAN (nur für RTP-MIDI nötig)
// ------------------------------------------------
//
// Die Zugangsdaten liegen in include/Credentials.h (nicht im Git).
// Vorlage: include/Credentials.example.h

#include "Credentials.h"

// ------------------------------------------------
// MIDI
// ------------------------------------------------

constexpr uint8_t MIDI_CHANNEL     = 1;
constexpr uint8_t DEFAULT_VELOCITY = 100;

// Anzahl der Sensoren
constexpr uint8_t NUM_SENSORS = 7;

// MIDI-Noten
constexpr uint8_t midiNotes[NUM_SENSORS] = {
    60, // C4
    62, // D4
    64, // E4
    65, // F4
    67, // G4
    69, // A4
    71  // H4
};

// Touch-Pins
//
// GPIO 1, 2, 3, 10, 11, 12, 13 sind auf den Stiftleisten des T-Display S3
// herausgeführt und touch-fähig (T1–T13), ohne Konflikt mit dem Display.
// Weitere interne Touch-Pins gibt es auf diesem Board nicht (GPIO 4–9
// und 14 sind durch Batterie-Messung, Display und Button belegt).
//
constexpr uint8_t touchPins[NUM_SENSORS] = {1, 2, 3, 10, 11, 12, 13};

// ------------------------------------------------
// Batterie
// ------------------------------------------------

// Batteriespannung, über 2:1-Spannungsteiler auf dem Board (ADC1,
// daher kein Konflikt mit aktivem WLAN)
constexpr uint8_t PIN_BAT_VOLT = 4;

// Messintervall in Millisekunden
constexpr uint32_t BATTERY_UPDATE_MS = 5000;

// ------------------------------------------------
// Touch-Kalibrierung (ESP32-S3)
// ------------------------------------------------
//
// Beim ESP32-S3 STEIGT der touchRead()-Wert bei Berührung (anders als
// beim klassischen ESP32). Beim Start wird pro Sensor eine Baseline
// gemessen; "gedrückt" gilt, wenn der Wert die Baseline um den
// jeweiligen Faktor übersteigt (Hysterese: ON-Faktor > OFF-Faktor).

constexpr float TOUCH_ON_RATIO  = 1.15f;
constexpr float TOUCH_OFF_RATIO = 1.08f;

constexpr uint8_t TOUCH_CALIBRATION_SAMPLES = 16;

// Baseline-Nachführung: solange ein Sensor NICHT gedrückt ist, folgt
// seine Baseline dem Messwert langsam (IIR-Tiefpass). Das kompensiert
// Drift durch austrocknendes Gemüse, Temperatur oder Kabellage.
//
// Alle TOUCH_BASELINE_INTERVAL_MS wird die Baseline um 1/FILTER der
// aktuellen Abweichung nachgezogen — mit 250 ms und Faktor 16 ergibt
// das eine Zeitkonstante von ~4 s: langsame Drift wird ausgeglichen,
// eine normale Berührung (deutlich kürzer) verschiebt fast nichts.
constexpr uint32_t TOUCH_BASELINE_INTERVAL_MS = 250; // 0 = Nachführung aus
constexpr uint16_t TOUCH_BASELINE_FILTER      = 16;

// ------------------------------------------------
// Buttons
// ------------------------------------------------

// Unterer Board-Button (aktiv LOW): manuelle Rekalibrierung aller
// Sensoren im Betrieb — z. B. nach dem Umstecken auf neues Gemüse.
// Während der Kalibrierung die Elektroden nicht berühren!
constexpr uint8_t PIN_BUTTON_RECALIBRATE = 14;
