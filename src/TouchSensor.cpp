#include "TouchSensor.h"

#include "Config.h"

TouchSensor::TouchSensor(uint8_t pin, uint8_t note)
    : _pin(pin)
    , _note(note)
    , _value(0)
    , _baseline(0)
    , _onThreshold(0)
    , _offThreshold(0)
    , _lastBaselineUpdate(0)
    , _pressed(false)
    , _pressedEvent(false)
    , _releasedEvent(false)
{
}

void TouchSensor::begin()
{
    recalibrate();
}

// Misst die Baseline neu und setzt die Schwellwerte. Blockiert für
// (TOUCH_CALIBRATION_SAMPLES + 1) * 10 ms — dabei nicht berühren!
void TouchSensor::recalibrate()
{
    // Erste Messung verwerfen (Sensor einschwingen lassen)
    touchRead(_pin);

    delay(10);


    uint64_t sum = 0;

    for (uint8_t i = 0; i < TOUCH_CALIBRATION_SAMPLES; i++)
    {
        sum += touchRead(_pin);

        delay(10);
    }


    _baseline = sum / TOUCH_CALIBRATION_SAMPLES;

    _onThreshold  = _baseline * TOUCH_ON_RATIO;
    _offThreshold = _baseline * TOUCH_OFF_RATIO;


    // Zustand zurücksetzen: eine vor der Kalibrierung gehaltene Note
    // erzeugt danach kein verwaistes Release-Event mehr. Das NoteOff
    // dafür verschickt der Aufrufer VOR recalibrate() (siehe main.cpp).
    _pressed       = false;
    _pressedEvent  = false;
    _releasedEvent = false;

    _lastBaselineUpdate = millis();
}

void TouchSensor::update()
{
    _value = touchRead(_pin);


    _pressedEvent  = false;
    _releasedEvent = false;


    if (!_pressed && _value > _onThreshold)
    {
        _pressed      = true;
        _pressedEvent = true;
    }

    if (_pressed && _value < _offThreshold)
    {
        _pressed       = false;
        _releasedEvent = true;
    }


    // Baseline-Nachführung: nur im losgelassenen Zustand und nur alle
    // TOUCH_BASELINE_INTERVAL_MS einen Filterschritt — so wird langsame
    // Drift (austrocknendes Gemüse, Temperatur) ausgeglichen, während
    // eine normale Berührung die Baseline praktisch nicht bewegt.
    if (!_pressed && TOUCH_BASELINE_INTERVAL_MS > 0
        && millis() - _lastBaselineUpdate >= TOUCH_BASELINE_INTERVAL_MS)
    {
        _lastBaselineUpdate = millis();

        _baseline = (_baseline * (TOUCH_BASELINE_FILTER - 1) + _value) / TOUCH_BASELINE_FILTER;

        _onThreshold  = _baseline * TOUCH_ON_RATIO;
        _offThreshold = _baseline * TOUCH_OFF_RATIO;
    }
}

bool TouchSensor::pressedEvent()
{
    return _pressedEvent;
}

bool TouchSensor::releasedEvent()
{
    return _releasedEvent;
}

bool TouchSensor::isPressed()
{
    return _pressed;
}

uint8_t TouchSensor::velocity()
{
    return DEFAULT_VELOCITY;
}

uint32_t TouchSensor::value()
{
    return _value;
}

uint32_t TouchSensor::baseline()
{
    return _baseline;
}

uint8_t TouchSensor::note()
{
    return _note;
}
