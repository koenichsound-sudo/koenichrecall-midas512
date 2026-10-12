# Koenich Recall – MIDAS 512 EQ V1.0.1

Hardware-Recall als JUCE/C++-VST3 für REAPER auf macOS Apple Silicon. **Dieses Plugin verändert kein Audio.** Auch IN und BELL sind ausschließlich gespeicherte Hardware-Einstellungen. Es steuert kein angeschlossenes MIDAS-Gerät.

## Download und Repository

Dieses Repository enthält den Quellcode, die Tests und die Build-Anleitung. Die kompilierte VST3 wird separat als GitHub-Release-Datei `KoenichRecall-MIDAS512-v1.0.1-macOS-arm64.zip` bereitgestellt. Das Repository benötigt keine Referenzbilder oder externen Grafikdateien.

## Installation der fertigen VST3

1. REAPER schließen.
2. `Koenich Recall - MIDAS 512 EQ.vst3` aus dem Lieferpaket in `~/Library/Audio/Plug-Ins/VST3/` kopieren. Den Ordner bei Bedarf erstellen. Finder → Gehe zu → Gehe zum Ordner hilft beim Öffnen.
3. REAPER starten und im FX-Browser nach `Koenich Recall` suchen. Falls erforderlich unter Einstellungen → Plug-ins → VST erneut scannen.
4. Plugin auf einer Spur einsetzen, Hardwarepositionen eintragen und das REAPER-Projekt speichern. Beim Wiederöffnen stellt der Host den gespeicherten Zustand wieder her.

Das Paket enthält einen lokalen ARM64-Build, keine Intel-Version. Der Build ist ad-hoc signiert, nicht mit einem Apple-Entwicklerzertifikat signiert oder notarisiert. macOS kann heruntergeladene Kopien daher zunächst blockieren.

## Änderungen in V1.0.1

Width steht bei 0,5 Oktaven exakt auf 12 Uhr. Die Tooltip-Texte verwenden korrektes UTF-8. Bestehende gespeicherte Werte und Hostautomation bleiben kompatibel; die Korrektur betrifft die Drehgeometrie.

## Bedienung

- Vier konzentrische Regler links: **Innenknopf = Gain**, **Außenring mit heller Markierung = Frequenz**. Die beim Drücken getroffene Fläche bleibt während des Ziehens zuständig.
- Vertikal ziehen; Shift für Feinregelung. Mausradänderungen sind deaktiviert, um versehentliche Recall-Änderungen zu vermeiden. Doppelklick setzt den jeweiligen Regler auf seinen unten dokumentierten Startwert.
- Die vier rechten Regler speichern Width. Treble- und Bass-Width bleiben auch im Shelf-Modus bedienbar und gespeichert.
- Zwei BELL-Taster: grün bei aktivem Bell-Zustand. IN: orange bei aktivem Zustand.
- Rechts lassen sich Zahlenwerte direkt eingeben. Frequenzen in Hz eingeben (z. B. `4000`, nicht `4k`).
- Reglerweg: 300°, Minimum ungefähr 7 Uhr, Maximum ungefähr 5 Uhr. Frequenzpositionen sind mathematisch logarithmisch. Gain ist linear. Die Width-Drehskala ist auf 0,5 Oktaven bei 12 Uhr kalibriert (Bereich 0,1–2; Skalenmarken 0,3 und 0,6). Die Host-Normalisierung bleibt für bestehende Automationsdaten unverändert.
- Fenstergröße lässt sich bei festem Seitenverhältnis ändern. Die vektorgezeichnete Frontplatte orientiert sich an den ursprünglichen Originalvorlagen. Beschriftungen/Skalen sind für die vereinbarte digitale Kennlinie neu gesetzt; dies ist keine pixelidentische Reproduktion des Fotos.

## Parameter und Startwerte

| Band | Gain | Frequenz | Width | Startwerte Gain / Frequenz / Width |
|---|---|---|---|---|
| Treble | −15…+15 dB | 1000…20000 Hz, logarithmisch | 0,1…2 Oktaven | 0 dB / 4000 Hz / 0,5 oct |
| Hi Mid | −15…+15 dB | 400…8000 Hz, logarithmisch | 0,1…2 Oktaven | 0 dB / 1600 Hz / 0,5 oct |
| Lo Mid | −15…+15 dB | 100…2000 Hz, logarithmisch | 0,1…2 Oktaven | 0 dB / 400 Hz / 0,5 oct |
| Bass | −15…+15 dB | 20…400 Hz, logarithmisch | 0,1…2 Oktaven | 0 dB / 80 Hz / 0,5 oct |

BELL startet bei Treble und Bass aus, IN an. Die Startwerte sind Software-Defaults, keine aus dem Produktfoto abgelesenen Hardwarepositionen.

Die 15 stabilen Parameter-IDs stehen in `Source/PluginProcessor.h`. Alle Parameter sind über JUCE AudioProcessorValueTreeState hostautomatisierbar und werden im Plugin-State gespeichert. Die Oberfläche und die Zahlenfelder folgen auch Hostautomation und geladenen Zuständen. Eine von JUCE/Host zusätzlich angebotene Bypass-Funktion zählt nicht zu den 15 Recall-Parametern.

## Selbst bauen

Voraussetzungen: Apple Silicon Mac, macOS 12 oder neuer, Xcode Command Line Tools, CMake ab 3.22, Git. Das Projekt verwendet JUCE **8.0.15** und C++17. Beim ersten Konfigurieren wird JUCE automatisch von dessen offiziellem GitHub-Repository geladen; dazu ist Internetzugang nötig.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_ARCHITECTURES=arm64
cmake --build build --config Release --parallel 4
ctest --test-dir build --output-on-failure
```

Mit bereits vorhandenem JUCE-Quellcode funktioniert der Build offline:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DJUCE_SOURCE_DIR=/absoluter/Pfad/zu/JUCE
cmake --build build --config Release --parallel 4
```

Die Plugin-Datei liegt anschließend unter:

`build/KoenichRecallMIDAS512_artefacts/Release/VST3/Koenich Recall - MIDAS 512 EQ.vst3`

Tests mit GUI-Vorschau und Laden der tatsächlich gebauten VST3 (absolute Pfade einsetzen):

```sh
build/RecallTests_artefacts/Release/RecallTests "/absoluter/Pfad/Vorschau.png" "/absoluter/Pfad/Koenich Recall - MIDAS 512 EQ.vst3"
```

Die Tests verwenden macOS-GUI-Initialisierung und sind für eine angemeldete Desktop-Sitzung gedacht. `-DRECALL_BUILD_TESTS=OFF` baut nur das Plugin. CMake installiert das Plugin absichtlich nicht automatisch in systemweite Ordner.

## Audio und State

Mono, Stereo sowie identische Ein-/Ausgangslayouts bis 64 Kanäle werden akzeptiert. Unterschiedliche Ein-/Ausgangslayouts werden abgelehnt. Der Audio-Callback verändert keine Samples, liest keine Recall-Parameter und verwendet weder Locks noch Speicherallokationen. Float und Double werden unterstützt; Latenz und Tail betragen null.

State-Schema 1 enthält alle 15 Werte. Unvollständige Zustände, fremde Schema-Versionen, unbekannte/duplizierte IDs und ungültige Werte werden vor der Übernahme abgelehnt. Parameter-IDs dürfen für spätere Versionen nicht umbenannt werden, damit Projekte kompatibel bleiben.

## Dateien und Rechte

- `Source/`: vollständige Plugin- und GUI-Quelldateien.
- `Tests/`: Prozessortests, GUI-Synchronisierung und VST3-Ladetest.
- `ThirdParty/JUCE-LICENSE.md`: JUCE-Lizenzinformationen. JUCE wird separat bezogen; vor Weitergabe oder kommerziellem Einsatz die zutreffende JUCE-Lizenz klären.

MIDAS ist eine Marke des jeweiligen Rechteinhabers. Dieses unabhängige Recall-Werkzeug ist kein offizielles MIDAS-Produkt.
