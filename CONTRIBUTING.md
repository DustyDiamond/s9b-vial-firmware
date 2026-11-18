# Contributing / Mitmachen

**Language / Sprache:** Issues, pull requests and comments in **English or German** only.
Contributions in other languages may be closed. /
Issues, Pull Requests und Kommentare bitte nur auf **Deutsch oder Englisch**. Beiträge in anderen
Sprachen werden ggf. geschlossen.

**Never post secrets / Niemals Geheimnisse posten:** no passwords, macro contents, `.vil` files or
`CURRENT.UF2` backups — they contain macros in plain text. For typing problems use a dummy string
such as `Aa1!Zz@{[]}\~|^`. / Keine Passwörter, Makroinhalte, `.vil`-Dateien oder
`CURRENT.UF2`-Sicherungen — sie enthalten Makros im Klartext. Für Tippfehler einen Dummy-Text wie
`Aa1!Zz@{[]}\~|^` verwenden.

**Security issues / Sicherheitsprobleme:** open an issue without exploit details and ask for a
private channel. / Issue ohne Details zum Angriff anlegen und nach einem privaten Kanal fragen.

## Pull requests

Open pull requests against `main`. GitHub Actions must build them green before merging
(squash or rebase, linear history). / Pull Requests gegen `main`; vor dem Merge muss der Build
von GitHub Actions grün sein.

1. Build and test as in [docs/BUILDING.md](docs/BUILDING.md): `tests/run.sh`, `scripts/build.sh`.
2. Changed the EEPROM layout (layers, macros, tap dances, Vial features, QMK settings)? Bump
   [`eeprom-layout.id`](eeprom-layout.id) and say so in [`CHANGELOG.md`](CHANGELOG.md).
3. Say how you tested (device, OS, keyboard layout).
4. Used AI tools? Say which — this project is developed with Claude Code and is open about it.
   / KI-Werkzeuge genutzt? Bitte angeben — dieses Projekt entsteht mit Claude Code und sagt das offen.

Contributions are licensed under [GPL-2.0-or-later](LICENSE), like the rest of the project. /
Beiträge stehen wie das übrige Projekt unter [GPL-2.0-or-later](LICENSE).
