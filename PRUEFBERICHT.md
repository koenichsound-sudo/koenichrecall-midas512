# Prüfbericht – V1.0.1

Erstellt am 12. Oktober 2026. Ziel: macOS Apple Silicon ARM64, Deployment-Target macOS 12.0. Gebaut mit Apple Clang 17, CMake 4.4 und JUCE 8.0.15 im Release-Modus.

## Korrekturen in V1.0.1

Alle vier Width-Regler: 0,5 Oktaven liegt exakt in der Mitte des 300°-Drehwegs, also bei 12 Uhr. Die neue GUI-Kennlinie ist invertierbar; Parameter-Normalisierung und absolute gespeicherte Werte bleiben unverändert. Die Frontplattenskala zeigt 0,1 / 0,3 / 0,6 / 2 passend zur neuen Geometrie. Das Minimum 0,1 entspricht der Originalabbildung.

BELL- und IN-Tooltips sowie Reglerhinweise verwenden explizites UTF-8. Tests prüfen die Width-Mittelposition, die inverse GUI-Kennlinie und das Wort „Grün“ im BELL-Tooltip.

## Automatisch geprüft

- Vollständiges CMake-Projekt kompiliert; echtes VST3-Bundle erzeugt.
- 15 Recall-Parameter, stabile IDs, korrekte Minima/Maxima und invertierbare Kennlinien.
- Logarithmische Frequenzmitten entsprechen dem geometrischen Mittel des jeweiligen Bereichs.
- State-Roundtrip mit Minima, Maxima und unterschiedlichen Zwischenwerten; Taster jeweils mit gültigen Ein/Aus-Werten.
- Ungültige Daten (beschädigter Chunk, NaN, unlesbare Werte und Werte außerhalb des Bereichs) verändern den vorhandenen Zustand nicht.
- Knopf → Hostparameter → Zahlenfeld sowie Zahlenfeld → Knopf und Host → GUI geprüft.
- Änderungen am Gain lassen die Frequenz unverändert und umgekehrt. Geprüfte Trefferflächen: Mitte trifft Gain, Außenring trifft Frequenz, Rechteckecken sind nicht aktiv.
- Beide BELL-Taster und IN synchronisieren ihre Zustände mit den Parametern.
- State-Wiederherstellung bei geöffnetem Editor aktualisiert alle Regler.
- Identische Ein-/Ausgangslayouts mit 1, 2, 8 und 64 Kanälen akzeptiert; abweichende Layouts abgelehnt.
- Float- und Double-Audio bitgenau unverändert: 44,1 / 48 / 96 kHz, 0 / 1 / 64 / 1024 Samples pro Block, IN an und aus. Tests enthalten auch negatives Null, einen Subnormalwert und Unendlich.
- Fertiges VST3-Bundle in JUCE-VST3-Host geladen: Scan, zwei Instanzen, State-Übertragung, Stereo-Audioverarbeitung und nativen Editor erfolgreich geprüft.
- Architekturprüfung: Mach-O ARM64. Abschließende ad-hoc-Bundlesignatur überprüft.
- GUI-Vorschau aus dem echten JUCE-Editor gerendert und visuell geprüft.

Die Tests befinden sich vollständig unter `Tests/RecallTests.cpp`; das Ergebnisprotokoll liegt in `TESTERGEBNIS.txt`.

## Grenzen dieser Prüfung

Der VST3-Ladetest erfolgte in einem JUCE-Testhost, nicht in REAPER. Speichern und erneutes Öffnen eines echten REAPER-Projekts sowie die Bedienung mit der Maus im REAPER-Fenster wurden nicht manuell geprüft. Die GUI-Tests prüfen die Parameterbindungen und Trefferflächen programmgesteuert. Die Frontplatte wurde anhand der bereitgestellten Bilder als Vektorgrafik nachgebaut; sie ist keine pixelidentische Fotoreproduktion.

Es gibt keine Developer-ID-Signatur und keine Apple-Notarisierung. Das Plugin wurde nicht automatisch in den persönlichen VST3-Ordner installiert.

## Kurzer REAPER-Abnahmetest

1. VST3 installieren und auf eine Spur laden.
2. Jeden Innenknopf und Außenring einzeln bewegen; der jeweils andere Wert muss unverändert bleiben.
3. Beide BELL-Taster einschalten (grün), IN ausschalten und wieder einschalten (orange).
4. Für alle Bänder unterschiedliche Werte eintragen; Projekt speichern, schließen und erneut öffnen.
5. Alle zwölf Zahlenwerte und drei Tasterzustände vergleichen. Das Audio muss unabhängig davon unverändert klingen; die technische Bitidentität wurde automatisiert geprüft.
