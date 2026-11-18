# S9B Vial Firmware

Open-Source-Firmware auf Basis von [Vial](https://get.vial.today/)/[QMK](https://qmk.fm/) für das
**9-Tasten-Makropad „S9B“ von Koolertron** (APM32F103, YRKB-UF2-Bootloader), gebaut für PCs
mit **deutscher Tastaturbelegung**. Sie ersetzt die Herstellerfirmware und bringt:

- **Makrotext kommt 1:1 an** auf einem deutschen PC (QWERTZ): `y z @ - { } [ ] \ | ~ ^` und alle
  anderen druckbaren ASCII-Zeichen, genau so, wie sie im Makro-Editor von Vial stehen.
- **PIN-Sperre direkt am Pad**: Sperren und Entsperren mit 5-stelliger PIN, ohne Software am PC.
  Gesperrt tippt das Pad nichts, und Vial kann es weder auslesen noch ändern.
- **LED-Rückmeldung**: Sperrzustand, PIN-Fortschritt, falsche PIN, Wartezeit, ein Farbton je Ebene.
- **Mehr Platz**: 8 Ebenen, 64 Makros, 16 Tap-Dances, einstellbare Tippgeschwindigkeit.

[English version](README.md) · [Download](../../releases/latest) · [Selbst bauen](docs/BUILDING.de.md) · [Changelog](CHANGELOG.md)

> [!NOTE]
> **Mit Claude Code gebaut.** Der Großteil dieses Projekts — die Analyse der Herstellerfirmware,
> der Firmware-Code, die Tests, das Build-Werkzeug und die Dokumentation — wurde von
> [Claude Code](https://claude.com/claude-code), dem KI-Coding-Agenten von Anthropic, geschrieben,
> und die Weiterentwicklung läuft genauso. Ein Mensch hat die Anforderungen gestellt, entschieden
> und jede Version auf einem echten Gerät getestet. Siehe [Wie dieses Projekt entsteht](#wie-dieses-projekt-entsteht).

> [!WARNING]
> Das Flashen eigener Firmware geschieht auf eigene Gefahr und kann Makros löschen oder das Pad
> unbrauchbar machen. Vorher den [Haftungsausschluss](#haftungsausschluss) lesen und **die
> Sicherung aus Schritt 3 unten** anlegen.
>
> **Vor jedem Flashen, auch vor Updates:** die Belegung in Vial mit *File → Save current layout*
> sichern (eine `.vil`-Datei, JSON). `s9b-update.uf2` soll Belegung und Makros zwar behalten, aber
> nur mit Sicherung bekommst du sie zurück, falls doch etwas schiefgeht. Die Datei enthält die
> Makros im Klartext, also wie ein Passwort aufbewahren.

---

## Passt mein Pad?

Getestet mit **einem** Gerät: Koolertron Einhand-Tastatur mit 9 Tasten, Amazon.de ASIN `B07VY2HKFZ`.
Dein Pad ist sehr wahrscheinlich dieselbe Hardware, wenn **alles** hiervon zutrifft:

- Vial (0.7.x) zeigt es als **„S9B“**, Hersteller **„MYKB“**; der Windows-Geräte-Manager zeigt
  die USB-ID **`A286:1DAD`**.
- Ein Druck auf eine in Vial mit **Reset** belegte Taste lässt ein Laufwerk **`YRKB_UPDATE`**
  erscheinen, darauf eine Datei `CURRENT.UF2`.
- Auf der Platine sitzt ein **Geehy APM32F103CB** (oder STM32F103CB) mit 9 RGB-LEDs.

Neuere Revisionen (die Produktfotos zeigen eine „V2“) können andere Pins haben. Gehen nach dem
Flashen Tasten oder LEDs nicht, die Sicherung zurückspielen (siehe unten) und ein Issue anlegen.

## Installation (erstes Mal, von der Herstellerfirmware)

Du brauchst [Vial](https://get.vial.today/) ab 0.7.5 und einen PC (Windows, macOS oder Linux).

1. **Makros notieren.** Sie werden gelöscht. (Vial: *File → Save current layout* speichert eine
   `.vil`-Kopie, die du nach dem Wechsel wieder laden kannst. Sie enthält die Makros im Klartext.)
2. **Bootloader starten:** in Vial die Taste **Reset** (Reiter *Quantum*, teils `QK_BOOT` genannt)
   auf eine beliebige Taste legen und drücken. Das Laufwerk **`YRKB_UPDATE`** erscheint.
3. **Herstellerfirmware sichern:** `CURRENT.UF2` vom Laufwerk an einen sicheren Ort kopieren. Das
   ist der komplette Flash-Inhalt deines Pads, **samt Hersteller-Makros im Klartext** — wie ein
   Passwort aufbewahren (z. B. als Anhang im Passwortmanager). Das ist dein Weg zurück.
4. **`s9b-first-install.uf2`** aus dem [aktuellen Release](../../releases/latest) laden (optional mit
   `SHA256SUMS` prüfen; der Build ist [reproduzierbar](docs/BUILDING.de.md)) und auf das Laufwerk `YRKB_UPDATE` kopieren.
5. Das Pad startet mit der neuen Firmware neu. Die LEDs leuchten grün; Vial zeigt jetzt „S9B“
   mit 8 Ebenen. Ab Werk spielt Taste *n* das Makro `M(n−1)`.

## Update

1. Pad entsperren (falls gesperrt).
2. **Belegung sichern:** Vial → *File → Save current layout* (`.vil`, JSON mit Belegung, Makros und
   Einstellungen). Vor jedem Update, auch wenn `s9b-update.uf2` alles behalten sollte.
   Zurückspielen: *File → Load saved layout*.
3. Die mit **Reset** / `QK_BOOT` belegte Taste drücken (einmal in Vial belegen, z. B. auf einer
   höheren Ebene) oder die Notfall-Kombination nutzen (löscht alles, siehe unten).
4. **`s9b-update.uf2`** auf das Laufwerk `YRKB_UPDATE` kopieren.

Belegung, Makros und PIN sollten erhalten bleiben, außer das [Changelog](CHANGELOG.md) sagt etwas anderes.

## Zurück zur Herstellerfirmware

Bootloader starten und die gesicherte `CURRENT.UF2` auf das Laufwerk `YRKB_UPDATE` kopieren. Den
Bootloader selbst überschreibt keine dieser Dateien.

> Das Zurückspielen der Sicherung ist der übliche UF2-Weg; am Testgerät wurde es bisher
> **nicht** ausprobiert.

---

## Bedienung

### Tastennummern

PIN-Ziffern und Beschreibungen folgen dem Ziffernblock-Aufdruck der Tasten:

```
┌───┬───┬───┐
│ 7 │ 8 │ 9 │
├───┼───┼───┤
│ 4 │ 5 │ 6 │
├───┼───┼───┤
│ 1 │ 2 │ 3 │
└───┴───┴───┘
```

### PIN-Sperre

| Was | Wie |
|-----|-----|
| **Sperren** | **1** (unten links) und **9** (oben rechts) **1 s** halten, loslassen. LEDs werden schwach rot. |
| **Entsperren** | Die 5-stellige PIN tippen. LEDs blinken grün. |
| **PIN festlegen** (erstes Mal) | Wie Sperren. Ohne PIN startet die Einrichtung: PIN eingeben (blau), wiederholen (lila). Danach ist das Pad gesperrt. |
| **PIN ändern** | Entsperrt 1 + 9 **5 s** halten (LEDs werden blau), loslassen, neue PIN zweimal eingeben. |
| **Einrichtung abbrechen** | 30 s nichts drücken. Die alte PIN bleibt gültig. |
| **Notfall: alles löschen** | 1 + 9 **beim Einstecken** halten. Belegung, Makros und PIN werden gelöscht, dann startet der Bootloader. |

Die PIN hat 5 Ziffern von 1 bis 9 (keine 0, Wiederholungen erlaubt). Der Sperrzustand übersteht
das Abstecken: entsperrt bleibt entsperrt (zu Hause), gesperrt bleibt gesperrt bis zur PIN-Eingabe.

**Falsche PIN:** Versuch 1 und 2 sind frei. Ab dem 3. Fehlversuch läuft eine Wartezeit (30 s,
dann je weiterem Fehlversuch verdoppelt, höchstens 60 min), in der Eingaben ignoriert werden.
Gelöscht wird nie etwas. Abstecken startet eine laufende Wartezeit neu, also einfach abwarten.

### LEDs

| Anzeige | Bedeutung |
|---------|-----------|
| Eigene Farbe (in Vial eingestellt) | Entsperrt |
| Leicht verschobener Farbton | Höhere Ebene aktiv (≈14° je Ebene) |
| **Schwach rot** | Gesperrt |
| Weißer Blitz + eine weiße LED mehr | Ziffer angenommen (weiße LEDs = bisher eingegebene Ziffern) |
| **Grün**, 2× blinken | PIN richtig / neue PIN gespeichert |
| **Rot**, 3× blinken | PIN falsch / Wiederholung passt nicht |
| **Orange, pulsierend** | Wartezeit nach Fehlversuchen |
| Kurzer roter Blitz | Taste während der Wartezeit, oder Eingabe nach 5 s Pause verworfen |
| **Blau** / **lila** | PIN-Einrichtung: neue PIN eingeben / wiederholen |
| Hellrot / hellblau beim Halten von 1 + 9 | Jetzt loslassen = sperren / neue PIN festlegen |

### Makros in Vial

1. Pad entsperren. Zeigt Vial *„Press and hold the following keys…“*, **keine Tasten halten —
   die PIN am Pad tippen**; der Dialog schließt sich, sobald die PIN angenommen ist.
2. Reiter *Macros*: eine **Text**-Aktion anlegen und den Text genau so tippen, wie er ankommen soll.
3. Reiter *Keymap*: das Makro (`M0`…`M63`) auf eine Taste legen. Im Texteditor testen.

**Am PC muss die deutsche Belegung aktiv und die Feststelltaste aus sein.** Anmeldebildschirme,
Remote-Desktops oder virtuelle Maschinen mit US-Belegung tippen andere Zeichen.

**Text-Aktionen** nehmen alle druckbaren ASCII-Zeichen:

```
A-Z a-z 0-9 Leerzeichen ! " # $ % & ' ( ) * + , - . / : ; < = > ? @ [ \ ] ^ _ ` { | } ~
```

Zeichen außerhalb von ASCII (`€ µ ä ö ü Ä Ö Ü ß § ° ² ³ ´`) werden aus Text-Aktionen entfernt.
Stattdessen als **Tap**-Aktion einfügen: `ä ö ü ß` liegen im Reiter *Basic*, und der Reiter *User*
hat Tasten, die auf einem deutschen PC richtig ankommen:

```
€  @  {  }  [  ]  \  |  ~  µ  ²  ³  §  °  ´  `  ^  Ä  Ö  Ü
```

Tipp für generierte Passwörter: alle ASCII-Gruppen des Passwortmanagers sind in Ordnung; nur
„Erweitertes ASCII“ ausschalten.

### Tippgeschwindigkeit

Vial → *QMK Settings* → *Tap-Hold* → **Tap Code Delay** (ms je Tastenereignis, Standard 10) → *Save*.
Höher stellen, wenn Anmeldebildschirme oder Remote-Sitzungen Zeichen verschlucken.

### Ebenen, Tap-Dance, Farbe

- **8 Ebenen:** `MO(1)` (halten), `TG(1)` (umschalten) oder `TO(1)` (wechseln) auf eine Taste von Ebene 0 legen.
- **64 Makros**, zusammen etwa 3,5 KB; Vial zeigt die Belegung unten im Reiter *Macros*.
- **Tap-Dance** (16): eine Taste, verschiedene Aktionen für Tippen, Halten, Doppeltippen, Tippen-und-Halten.
- **Combos** sind aus, weil 1 + 9 die Sperrgeste ist.
- **Farbe:** Reiter *Lighting*, auf Ebene 0 einstellen; höhere Ebenen verschieben den Farbton von selbst.
- Vial kann Makros nicht benennen; die Zuordnung „Makronummer → Zweck“ im Passwortmanager notieren.

### Konverter für Nicht-ASCII-Text (optional)

[`tools/ConvertTo-VialMacroDE.ps1`](tools/ConvertTo-VialMacroDE.ps1) (PowerShell) wandelt beliebigen
Text, auch mit `€ ä ö ü ß`, in eine Vial-Tastenfolge für einen deutschen PC. Läuft lokal, liest den
Text verdeckt ein und legt das Ergebnis nur in die Zwischenablage. In Vial: Makro →
*Open Text Editor…* → einfügen → *Apply* → *Save*. Funktioniert auch mit der Herstellerfirmware.

---

## Sicherheitshinweise

Die PIN-Sperre ist eine Komfortfunktion, keine zertifizierte Sicherheit.

**Sie schützt vor:** jemandem, der auf einem gefundenen oder unbeaufsichtigten Pad Tasten drückt;
Auslesen oder Ändern von Belegung/Makros per Vial/VIA an einem anderen PC, solange gesperrt;
dem Sprung in den Bootloader im gesperrten Zustand (der Notfallweg löscht vorher alle Daten);
Raten (59.049 PINs, Wartezeit bis 60 min je Versuch).

**Sie schützt nicht vor:** Öffnen des Gehäuses und Auslesen des Flash über den SWD-Anschluss —
Makros liegen im Klartext, ein Ausleseschutz ist nicht gesetzt; Schadsoftware auf einem PC,
während das Pad entsperrt ist. Vial-Sicherungen (`.vil`) und `CURRENT.UF2` enthalten deine Makros
im Klartext. Kläre, ob Passwörter auf einem Makropad bei deinem Arbeitgeber erlaubt sind.

Sicherheitsproblem gefunden? Bitte ein Issue ohne sensible Details anlegen oder den Maintainer
über GitHub kontaktieren.

## Fehlersuche

| Problem | Ursache / Abhilfe |
|---------|-------------------|
| Tasten tun nichts, LEDs schwach rot | Gesperrt → PIN tippen. |
| Vial zeigt Fehler oder leere Belegung | Gesperrt → entsperren, Vial neu verbinden. |
| LEDs pulsieren orange | Wartezeit → abwarten, **nicht** abstecken. |
| Falsche Zeichen am PC | Belegung am PC nicht deutsch, Feststelltaste an oder Nicht-ASCII-Text in einer Text-Aktion. |
| `QK_BOOT` tut nichts | Geht nur entsperrt. |
| PIN vergessen / Pad unbrauchbar | 1 + 9 beim Einstecken halten → alles gelöscht → `s9b-update.uf2` erneut kopieren. |
| Alte `.vil` lädt mit Warnung „anderes Keyboard“ | Überspringen. Danach jedes Makro testen: Umwege für die Herstellerfirmware (z. B. `y` statt `z`) kommen jetzt wörtlich an. |

## Technische Daten

Siehe [README.md → Technical details](README.md#technical-details).

---

## Wie dieses Projekt entsteht

Dieses Projekt wird mit [Claude Code](https://claude.com/claude-code) entwickelt, dem
KI-Coding-Agenten von Anthropic, und das bleibt so:

- **Claude Code** hat Platine und Herstellerfirmware analysiert, QMK, Vial und den Bootloader
  recherchiert und Firmware, Patches, Tests, Build-Skripte, CI, den Konverter und diese
  Dokumentation geschrieben.
- **DustyDiamond** (Maintainer) hat die Anforderungen gestellt, die Entscheidungen getroffen,
  jede Version am echten Gerät getestet und entscheidet, was veröffentlicht wird.
- Commits tragen einen `Co-Authored-By: Claude`-Vermerk.

Heißt: Der Code wurde von einer Person geprüft und an einem Gerät getestet. Entsprechend
behandeln, vor dem Anvertrauen von Geheimnissen lesen und Probleme melden.

## Mitmachen

Issues und Pull Requests sind willkommen, auf **Deutsch oder Englisch** — siehe [CONTRIBUTING.md](CONTRIBUTING.md).
Niemals Passwörter, Makroinhalte oder `.vil`-Dateien posten.

Entwickelt wird hier auf GitHub: Jede Änderung, auch die des Maintainers, läuft über einen Pull
Request, den GitHub Actions baut und testet, bevor er gemergt wird. Releases entstehen aus
Versions-Tags (siehe [docs/BUILDING.de.md](docs/BUILDING.de.md)).

## Haftungsausschluss

Dies ist ein inoffizielles Hobbyprojekt, **ohne Verbindung zu** Koolertron, Geehy,
STMicroelectronics, QMK oder Vial und nicht von ihnen unterstützt. Alle Produktnamen und Marken
gehören ihren Inhabern.

Firmware, Werkzeuge und Dokumentation werden **ohne jede Gewährleistung** bereitgestellt, so wie
sie sind (siehe Abschnitte 11 und 12 der [Lizenz](LICENSE)). Das Flashen eigener Firmware kann das
Gerät unbrauchbar machen, die Garantie kosten oder gespeicherte Daten löschen, auch Makros und
Passwörter. Die Nutzung erfolgt **auf eigene Gefahr**. Soweit gesetzlich zulässig, haften die
Autoren und Mitwirkenden nicht für Schäden, Datenverlust, gesperrte Konten oder andere Folgen der
Nutzung. Die Haftung für Vorsatz und grobe Fahrlässigkeit sowie nach zwingenden gesetzlichen
Vorschriften bleibt unberührt.

## Lizenz

[GNU General Public License v2.0 oder später](LICENSE) (`GPL-2.0-or-later`), dieselbe Lizenz wie
QMK und Vial. Die veröffentlichte Firmware enthält Code von QMK, Vial und ChibiOS unter deren
Lizenzen; der zugehörige Quellcode ist dieses Repository plus vial-qmk im Commit aus
[`vial-qmk.ref`](vial-qmk.ref). Verbindlich ist der englische Lizenztext.

**Dank an** [QMK](https://github.com/qmk/qmk_firmware), [Vial](https://github.com/vial-kb) und
die Autoren des [YRKB-UF2-Bootloaders](https://github.com/Oh-My-Mechanical-Keyboard/kb_bootloaders).
