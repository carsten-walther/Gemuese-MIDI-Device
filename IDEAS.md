Was jetzt noch geht, sortiert nach Themen und grob nach Aufwand/Nutzen:

## Klang & Musikalität (der größte Spielraum)

**Wellenform-Umschalter** — ✅ Umgesetzt (über das Settings-Menü statt Boot-Button): Dreieck, Rechteck, Sägezahn, Sinus (1024er-LUT) und **8-Bit-Chiptune** (25-%-Puls mit 8-Bit-Rasterung in der Mix-Stufe, seit v1.0 der Default).

**Arpeggio** — ✅ Umgesetzt: gehaltene Akkorde werden im C64-Stil zyklisch als schnelle Notenfolge zerlegt (Off/Slow/Fast/Turbo, sample-genau im Audio-Task, mit De-Klick-Blende). Zusammen mit der 8-Bit-Welle der komplette 80er-Sound.

**Skalen und Oktav-Shift** — ✅ Umgesetzt (Dur, Moll, Pentatonik, Blues als Menüpunkt; Intervalltabellen in `include/Scales.h`) - sieben Pads sind fix auf C-Dur gemappt. Ein Skalenwahl-Modus (Dur, Moll, Pentatonik, Blues) plus Oktave hoch/runter macht das Instrument musikalisch vielseitiger, ohne Hardware zu ändern. Die Notentabelle wird dann zur Laufzeit berechnet statt aus der Config gelesen — und die Pad-Beschriftung zieht dank `noteName()` automatisch mit.

**Vibrato/Aftertouch** — die Touch-Werte liefern kontinuierlich Daten, auch *während* eine Note gehalten wird. Druckänderungen könnten Channel Pressure (MIDI) bzw. Tonhöhen- oder Lautstärkemodulation (Speaker) steuern. Das wäre das ausdrucksstärkste Feature überhaupt: die Gurke fester drücken → der Ton schwillt an.

**Sustain/Release-Verlängerung** am Speaker — aktuell klingt eine Note nach 40 ms Release ab. Ein längeres, konfigurierbares Ausklingen (oder ein simpler Hall via Delay-Line) würde den Klang deutlich weniger nach Piepser klingen lassen. Kostet nur RAM für den Delay-Buffer.

**Die Bleistift-Idee ist großartig** — und sie hat zwei Ausbaustufen, eine die sofort funktioniert und eine richtig spannende:

**Stufe 1** (funktioniert heute schon): Bleistiftstriche sind leitfähig genug für die kapazitive Messung. Du kannst dir also jetzt schon eine Klaviatur aufs Papier zeichnen, die Krokodilklemmen an die Strichenden klemmen — fertig. Der Strich wirkt als Elektrodenverlängerung, die Baseline-Kalibrierung frisst den Graphit-Offset, und sogar die Anschlagsdynamik funktioniert (mehr Fingerfläche auf dem Strich = höhere Velocity). Wichtig nur: satte, mehrfach nachgezogene Striche mit weichem Bleistift (B2 oder weicher), denn dünne HB-Striche haben Megaohm-Widerstände und koppeln schwach. Das wäre übrigens ein fantastisches README-Foto: gezeichnete Klaviatur statt Gemüse.

**Stufe 2** (deine eigentliche Idee — der Ribbon-Controller): Widerstand messen und daraus Töne modulieren. Physikalisch steckt da ein echtes Instrument drin: Ein langer, dicker Graphitstrich ist ein verteilter Widerstand (grob 50 kΩ–1 MΩ pro cm, je nach Härte und Deckung). Beschaltung im Makey-Makey-Stil: Strichende über einen ~1-MΩ-Pullup an 3,3 V und an einen ADC-Pin, der Spieler hält eine GND-Elektrode in der anderen Hand und berührt den Strich irgendwo — der ADC sieht einen Spannungsteiler aus Strichsegment + Körperwiderstand. Je weiter vom Clip entfernt du berührst, desto höher der Widerstand — ein kontinuierlicher Positionswert, ein gezeichnetes Ribbon.

Zwei ehrliche Einschränkungen, die das Design bestimmen: Erstens schwankt der Hautwiderstand massiv (trockene vs. feuchte Finger: Faktor 10) — der Absolutwert taugt daher nicht für präzise Tonhöhen, aber hervorragend für relative Modulation: den Finger auf dem Strich schieben → Pitch-Bend, Vibrato-Tiefe oder Filter. Genau da glänzt es, ein „Slide" klingt auf dem Chiptune-Synth herrlich. Alternativ grob quantisiert in 3–5 Zonen (mit Kalibrier-Geste), das ist robust. Zweitens die Pin-Frage, und die hat einen Haken: Alle Pins sind belegt, und von den Touch-Pins sind nur GPIO 1, 2, 3 und 10 ADC1-fähig — GPIO 11–13 hängen an ADC2, der sich mit aktivem WLAN beißt. Der saubere Weg wäre also: ein Pad opfern (z. B. GPIO 10), NUM_SENSORS auf 6, und dieser Pin wird per Config umschaltbar zum Ribbon-Eingang — sechs gezeichnete Tasten plus ein gezeichneter Pitch-Bend-Streifen darüber. Softwareseitig ein kleiner RibbonController: ADC oversampled, Median-Filter, Touch-Erkennung über Schwelle, Wert auf Bend/Vibrato gemappt — beim Speaker als Multiplikator auf den Phasenschritt, bei MIDI als echtes Pitch-Bend-Event.

Und was sonst noch ginge, in derselben kreativen Liga: Ein Noise-Kanal für den Synth (LFSR-Rauschen wie im NES — damit wird ein Pad zur Snare/HiHat und das BananaPhon zur kompletten Chiptune-Band; passt perfekt zum Arpeggio). Ein Looper — Encoder-Longpress startet die Aufnahme der gespielten Noten, danach loopt sie der Synth und du spielst darüber; das verwandelt das Gerät vom Instrument zur One-Person-Jam. Und die umgekehrte Richtung: BLE-MIDI-Empfang, sodass das BananaPhon als Klangerzeuger für einen externen Sequencer dient — die Infrastruktur dafür ist mit dem Synth ja komplett da.

Mein Vorschlag: Ich baue dir den Ribbon-Controller als Prototyp (Config-Schalter ENABLE_RIBBON, GPIO 10, Pitch-Bend auf Speaker + MIDI, Toast zeigt den Bend-Wert) — dann malst du einen dicken Strich und wir sehen, wie stabil die Werte auf echtem Papier sind, bevor wir Zonen oder Vibrato-Mapping verfeinern. Einverstanden, oder reizt dich einer der anderen Kandidaten mehr?

## Bedienung & Anzeige

**Einstellungen ohne Neu-Flashen** — ✅ Umgesetzt für Lautstärke, Wellenform, Arpeggio, Skala und Oktave: Settings-Menü am Rotary-Encoder (Klick = Parameter, Drehen = Wert), Ablage im NVS-Flash (Preferences), verzögertes Speichern. Noch in der `Config.h`: Touch-Schwellen und Velocity-Kennlinie — Kandidaten für weitere Menüpunkte, sobald der Bedarf da ist.

**Batterie-Warnung** — die Anzeige ist rein passiv. Unter ~10 % könnte das Batterie-Symbol blinken oder der Speaker einen dezenten Warnton spielen, bevor der LiPo in die Tiefentladung läuft.

**Splash-Screen** — ✅ Umgesetzt: Name + Firmware-Version beim Start (4 s), Kalibrierung und Funk-Initialisierung laufen währenddessen im Hintergrund.

## Robustheit & Strom

**Deep Sleep** — im Akkubetrieb läuft das Gerät, bis der LiPo leer ist. Nach z. B. 10 Minuten ohne Berührung in den Deep Sleep gehen und per Touch-Wakeup (der ESP32-S3 kann genau das) aufwachen — das verlängert die Akkulaufzeit von Stunden auf Wochen. Passt gut zusammen mit der Batterie-Warnung.

**Watchdog + Fehler-Resilienz** — der Audio-Task und die WiFiManager-Schleife laufen unbeaufsichtigt. Ein Task-Watchdog, der bei Hängern neu startet, plus ein Boot-Zähler (nach drei Crashs in Folge → Speaker aus, nur MIDI) wäre die Bühnen-Versicherung.

## Code & Infrastruktur

**Unit-Tests für die Logik** — Hüllkurve, Phasen-Akkumulator, Velocity-Kennlinie, Glitch-Filter und die Noten-Weiche sind pure Logik ohne Hardware. Mit PlatformIO's `pio test` (native environment) ließen die sich auf dem Rechner testen und in die bestehende CI hängen. Mein Sandbox-Stub war quasi der Vorläufer davon — als echtes `test/`-Verzeichnis im Repo wäre das dauerhaft wertvoll.

**Release-Workflow** — ein GitHub-Actions-Job, der bei einem Tag die Firmware baut und die `.bin` als Release-Asset anhängt. Zusammen mit ESP Web Tools (Flashen direkt aus dem Browser) könnten Nachbauer das Gerät flashen, ohne je PlatformIO zu installieren. Für ein Show-off-Projekt wie dieses ist das Gold.

**Doku-Feinschliff** — das Foto/GIF bleibt der offene Klassiker; dazu ein Schaltplan (auch handgezeichnet reicht) mit dem MAX98357A und den Krokodilklemmen, und optional ein 30-Sekunden-Video. Nichts davon ist Code, aber nichts würde dem Repo mehr Sterne bringen.