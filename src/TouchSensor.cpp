#include "TouchSensor.h"

#include "Config.h"

void TouchSensor::configure(uint8_t pin)
{
    _pin = pin;
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

    _measuring  = false;
    _aboveCount = 0;

    _lastBaselineUpdate = millis();
}

// Messwert relativ zur Baseline auf 0..127 abbilden. Gleiche Kennlinie
// wie die Velocity (ON-Schwelle -> 0, RATIO_MAX -> 127), aber über die
// volle MIDI-Spanne statt VELOCITY_MIN..MAX.
float TouchSensor::pressureFromValue(uint32_t value) const
{
    float ratio = static_cast<float>(value) / static_cast<float>(_baseline);

    float span = TOUCH_VELOCITY_RATIO_MAX - TOUCH_ON_RATIO;

    float t = span > 0.0f ? (ratio - TOUCH_ON_RATIO) / span : 1.0f;

    if (t < 0.0f)
    {
        t = 0.0f;
    }

    if (t > 1.0f)
    {
        t = 1.0f;
    }

    return t * 127.0f;
}

// Schließt das Peak-Fenster ab: Velocity aus dem Spitzenwert berechnen
// und das Press-Event auslösen.
void TouchSensor::finishMeasurement()
{
    _measuring = false;

    _pressed      = true;
    _pressedEvent = true;

    // Druck-Glättung auf dem Anschlagswert starten — sonst kröche der
    // Aftertouch nach jedem NoteOn erst von 0 hoch. Der Bezugspunkt
    // für die Modulation wird erst später festgehalten (siehe update).
    _pressureSmooth = pressureFromValue(_peak);
    _pressureRefSet = false;
    _pressStart     = millis();

    if (!ENABLE_TOUCH_VELOCITY)
    {
        _velocity = DEFAULT_VELOCITY;

        return;
    }

    // Peak relativ zur Baseline linear auf VELOCITY_MIN..VELOCITY_MAX
    // abbilden: ON-Schwelle -> MIN, RATIO_MAX -> MAX (mit Begrenzung).
    float ratio = static_cast<float>(_peak) / static_cast<float>(_baseline);

    float span = TOUCH_VELOCITY_RATIO_MAX - TOUCH_ON_RATIO;

    float t = span > 0.0f ? (ratio - TOUCH_ON_RATIO) / span : 1.0f;

    if (t < 0.0f)
    {
        t = 0.0f;
    }

    if (t > 1.0f)
    {
        t = 1.0f;
    }

    _velocity = VELOCITY_MIN + static_cast<uint8_t>(t * (VELOCITY_MAX - VELOCITY_MIN) + 0.5f);
}

void TouchSensor::update()
{
    _value = touchRead(_pin);


    _pressedEvent  = false;
    _releasedEvent = false;


    if (!_pressed && !_measuring)
    {
        // Glitch-Filter: erst nach TOUCH_CONFIRM_SAMPLES Messungen in
        // Folge über der ON-Schwelle beginnt der Anschlag — ein
        // einzelner Ausreißer löst keine Geisternote aus.
        _aboveCount = _value > _onThreshold ? _aboveCount + 1 : 0;

        if (_aboveCount >= TOUCH_CONFIRM_SAMPLES)
        {
            _aboveCount = 0;

            if (TOUCH_VELOCITY_WINDOW_MS == 0)
            {
                // Kein Fenster: sofort auslösen, Velocity aus dieser Messung
                _peak = _value;

                finishMeasurement();
            }
            else
            {
                _measuring    = true;
                _measureStart = millis();
                _peak         = _value;
            }
        }
    }
    else if (_measuring)
    {
        if (_value > _peak)
        {
            _peak = _value;
        }

        // Fenster abgelaufen — oder der Finger ist schon wieder weg
        // (sehr kurzer Tipper): in beiden Fällen jetzt auslösen, das
        // Release erledigt der Block darunter im selben Durchlauf.
        if (millis() - _measureStart >= TOUCH_VELOCITY_WINDOW_MS || _value < _offThreshold)
        {
            finishMeasurement();
        }
    }

    if (_pressed && _value < _offThreshold)
    {
        _pressed       = false;
        _releasedEvent = true;
    }

    // Aftertouch: bei gehaltener Taste den Druck laufend nachführen.
    // Der Rohwert zittert, deshalb ein IIR-Tiefpass — ohne ihn würde
    // die Modulation hörbar flattern.
    if (ENABLE_AFTERTOUCH && _pressed)
    {
        _pressureSmooth += (pressureFromValue(_value) - _pressureSmooth) / AFTERTOUCH_FILTER;

        // Bezugspunkt erst festhalten, wenn der Anschlag abgeklungen
        // und der Griff eingeschwungen ist — ab da moduliert nur noch,
        // was der Finger wirklich zusätzlich tut
        if (!_pressureRefSet && millis() - _pressStart >= AFTERTOUCH_SETTLE_MS)
        {
            _pressureRef    = pressure();
            _pressureRefSet = true;
        }
    }


    // Baseline-Nachführung: nur im losgelassenen Zustand und nur alle
    // TOUCH_BASELINE_INTERVAL_MS einen Filterschritt — so wird langsame
    // Drift (austrocknendes Gemüse, Temperatur) ausgeglichen, während
    // eine normale Berührung die Baseline praktisch nicht bewegt.
    if (!_pressed && !_measuring && TOUCH_BASELINE_INTERVAL_MS > 0 &&
        millis() - _lastBaselineUpdate >= TOUCH_BASELINE_INTERVAL_MS)
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
    return _velocity;
}

uint8_t TouchSensor::pressure()
{
    return static_cast<uint8_t>(_pressureSmooth + 0.5f);
}

int16_t TouchSensor::pressureDelta()
{
    if (!_pressureRefSet)
    {
        return 0;
    }

    return static_cast<int16_t>(static_cast<int>(pressure()) - static_cast<int>(_pressureRef));
}

uint32_t TouchSensor::value()
{
    return _value;
}

uint32_t TouchSensor::baseline()
{
    return _baseline;
}
