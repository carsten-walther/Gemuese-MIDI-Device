#pragma once

#include <Arduino.h>

class TouchSensor
{
public:
    TouchSensor(uint8_t pin, uint8_t note);

    void begin();
    void recalibrate();
    void update();
    bool pressedEvent();
    bool releasedEvent();
    bool isPressed();

    uint8_t velocity();
    uint32_t value();
    uint32_t baseline();
    uint8_t note();

private:
    uint8_t _pin;
    uint8_t _note;
    uint32_t _value;
    uint32_t _baseline;
    uint32_t _onThreshold;
    uint32_t _offThreshold;
    uint32_t _lastBaselineUpdate;

    bool _pressed;
    bool _pressedEvent;
    bool _releasedEvent;
};
