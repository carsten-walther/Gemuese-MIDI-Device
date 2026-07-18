#pragma once

#include <Arduino.h>

#include "Config.h"

class TouchSensor
{
public:
    TouchSensor() = default;

    // Pin zuweisen — vor recalibrate() aufrufen. Getrennt vom
    // Konstruktor, damit die Sensoren als statisches Array (ohne Heap)
    // angelegt und in setup() aus Config-Tabellen bestückt werden
    // können. Die MIDI-Note ergibt sich seit der Skalen-Umstellung
    // zur Laufzeit aus Skala + Oktave (siehe main.cpp).
    void configure(uint8_t pin);

    void recalibrate();
    void update();
    bool pressedEvent();
    bool releasedEvent();
    bool isPressed();

    uint8_t velocity();

    // Aktueller Anpressdruck 0..127 (geglättet). Nur bei gehaltener
    // Taste sinnvoll — für Aftertouch/Channel Pressure.
    uint8_t pressure();

    // Druckänderung gegenüber dem eingeschwungenen Griff (negativ =
    // lockerer). 0, solange der Bezugspunkt noch nicht steht — der
    // wird erst AFTERTOUCH_SETTLE_MS nach dem Anschlag festgehalten,
    // weil die Anschlagsspitze über dem gehaltenen Druck liegt.
    int16_t pressureDelta();

    uint32_t value();
    uint32_t baseline();

private:
    uint8_t _pin = 0;

    uint32_t _value              = 0;
    uint32_t _baseline           = 0;
    uint32_t _onThreshold        = 0;
    uint32_t _offThreshold       = 0;
    uint32_t _lastBaselineUpdate = 0;

    // Glitch-Filter: Zähler aufeinanderfolgender Messungen über der
    // ON-Schwelle (siehe Config.h)
    uint8_t _aboveCount = 0;

    // Peak-Fenster für die Anschlagsdynamik (siehe Config.h)
    bool _measuring        = false;
    uint32_t _measureStart = 0;
    uint32_t _peak         = 0;
    uint8_t _velocity      = DEFAULT_VELOCITY;

    // Aftertouch: geglätteter Druck, läuft nur bei gehaltener Taste,
    // plus der nach dem Einschwingen festgehaltene Bezugspunkt
    float _pressureSmooth = 0.0f;
    uint8_t _pressureRef  = 0;
    bool _pressureRefSet  = false;
    uint32_t _pressStart  = 0;

    void finishMeasurement();

    // Messwert auf 0..127 abbilden (gleiche Kennlinie wie die Velocity,
    // aber über die volle MIDI-Spanne)
    float pressureFromValue(uint32_t value) const;

    bool _pressed       = false;
    bool _pressedEvent  = false;
    bool _releasedEvent = false;
};
