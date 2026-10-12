# Koenich Recall – MIDAS 512 EQ v1.0.1

VST3-Recall-Plugin für macOS Apple Silicon (ARM64), macOS 12 oder neuer.

- Vier unabhängig bedienbare konzentrische Gain-/Frequenzregler.
- Vier Width-Regler, 0,1–2 Oktaven; 0,5 steht bei 12 Uhr.
- Zwei grüne BELL-Taster und ein orangefarbener IN-Taster.
- Alle 15 Einstellungen werden im Host-Projekt gespeichert.
- Unveränderte Audiodurchleitung ohne Klangbearbeitung.
- V1.0.1 korrigiert die Width-Drehskala und UTF-8-Tooltips; gespeicherte Werte und Hostautomation bleiben kompatibel.

## Installation

`KoenichRecall-MIDAS512-v1.0.1-macOS-arm64.zip` entpacken. REAPER schließen und das enthaltene VST3-Bundle nach `~/Library/Audio/Plug-Ins/VST3/` kopieren. Eine ältere Version ersetzen. REAPER neu starten und bei Bedarf neu scannen.

Der ARM64-Build ist ad-hoc signiert, nicht notarisiert. Kein Intel-Build. Automatisierte Tests prüfen Recall, GUI-Parameterbindungen, Audiodurchleitung und das Laden der VST3 im JUCE-Testhost. Die Darstellung von V1.0.1 wurde vom Nutzer bestätigt; ein vollständiger REAPER-Projekt-Roundtrip wurde nicht durch den Entwickler geprüft.
