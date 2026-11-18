# Copyright 2026 DustyDiamond
# SPDX-License-Identifier: GPL-2.0-or-later
<#
.SYNOPSIS
  Wandelt einen Text (z. B. ein Passwort) in ein Vial-Makro um, das an einem
  Windows-Host mit deutscher Tastaturbelegung (QWERTZ, T1) genau diesen Text tippt.

.DESCRIPTION
  - Läuft komplett lokal, ohne Netzwerkzugriff, ohne Logdatei, ohne Datei-Ausgabe.
  - Eingabe verdeckt (Read-Host -AsSecureString), zweimal zur Kontrolle.
  - Ergebnis (Vial-Makro-JSON) geht nur in die Zwischenablage, markiert als
    "nicht in den Verlauf / nicht in die Cloud-Zwischenablage" und wird nach
    Bestätigung wieder geleert. Auf der Konsole erscheint nichts vom Inhalt.
  - Zeichen, die auf US und DE auf derselben Taste liegen (a-x, A-X, 0-9,
    Leerzeichen, ! $ % , .), werden als Vial-"Text" ausgegeben (1 Byte/Zeichen),
    alle anderen als Down/Tap/Up-Folgen mit Basis-Keycodes (3 Byte je Aktion).
    Das funktioniert mit der normalen Vial-Firmware, ohne Flashen.

  Einfügen in Vial: Makro-Tab M<n> -> "Open Text Editor..." -> Inhalt markieren
  -> Strg+V -> "Apply" -> unten "Save".

.PARAMETER SelfTest
  Prüft die Umwandlung mit festen Dummy-Strings und gibt deren JSON aus.
  Nutzt keine Zwischenablage und fragt nichts ab.
#>
[CmdletBinding()]
param([switch]$SelfTest)

Set-StrictMode -Version 2
$ErrorActionPreference = 'Stop'

# Zeichen -> @(Keycode, Modifier, Totaste)
#   Modifier: '' = keiner, 'S' = Shift (KC_LSFT), 'A' = AltGr (KC_RALT)
#   Totaste:  $true = danach Leertaste tippen, damit das Zeichen selbst erscheint
function Add-Key([string]$chars, [string[]]$kcs, [string]$mod, [bool]$dead = $false) {
    for ($i = 0; $i -lt $chars.Length; $i++) {
        $DE[[string]$chars[$i]] = @($kcs[$i], $mod, $dead)
    }
}
# Buchstaben (Y/Z vertauscht), case-sensitiv per Ordinal-Vergleich (Hashtable ist case-insensitiv!)
$DE = New-Object 'System.Collections.Generic.Dictionary[string,object]' ([StringComparer]::Ordinal)
foreach ($c in [char[]]'abcdefghijklmnopqrstuvwxyz') {
    $kc = 'KC_' + ([string]$c).ToUpperInvariant()
    if ($c -eq [char]'y') { $kc = 'KC_Z' } elseif ($c -eq [char]'z') { $kc = 'KC_Y' }
    $DE[[string]$c] = @($kc, '', $false)
    $DE[([string]$c).ToUpperInvariant()] = @($kc, 'S', $false)
}
# Ziffernreihe
Add-Key '1234567890' @('KC_1','KC_2','KC_3','KC_4','KC_5','KC_6','KC_7','KC_8','KC_9','KC_0') ''
Add-Key ('!"' + [char]0x00A7 + '$%&/()=') @('KC_1','KC_2','KC_3','KC_4','KC_5','KC_6','KC_7','KC_8','KC_9','KC_0') 'S'
Add-Key ([string][char]0x00B2 + [char]0x00B3 + '{[]}') @('KC_2','KC_3','KC_7','KC_8','KC_9','KC_0') 'A'
# Restliche Tasten (Zeichen werden per Codepoint angegeben, damit die Datei kodierungsfest ist)
$sz = [string][char]0x00DF; $ue = [string][char]0x00FC; $oe = [string][char]0x00F6; $ae = [string][char]0x00E4
Add-Key ($sz + $ue + '+' + $oe + $ae + '#<,.- ') @('KC_MINS','KC_LBRC','KC_RBRC','KC_SCLN','KC_QUOT','KC_NUHS','KC_NUBS','KC_COMM','KC_DOT','KC_SLSH','KC_SPC') ''
Add-Key ('?' + $ue.ToUpperInvariant() + '*' + $oe.ToUpperInvariant() + $ae.ToUpperInvariant() + "'>;:_" + [char]0x00B0) @('KC_MINS','KC_LBRC','KC_RBRC','KC_SCLN','KC_QUOT','KC_NUHS','KC_NUBS','KC_COMM','KC_DOT','KC_SLSH','KC_GRV') 'S'
Add-Key ('\~|@' + [char]0x20AC + [char]0x00B5) @('KC_MINS','KC_RBRC','KC_NUBS','KC_Q','KC_E','KC_M') 'A'
# Totasten: ^ (KC_GRV), Akut (KC_EQL), Gravis/Backtick (Shift+KC_EQL)
Add-Key '^' @('KC_GRV') '' $true
Add-Key ([string][char]0x00B4) @('KC_EQL') '' $true
Add-Key '`' @('KC_EQL') 'S' $true

# Zeichen, die auf US- und DE-Layout identisch getippt werden -> als Vial-"Text" sicher
$TextSafe = 'abcdefghijklmnopqrstuvwxABCDEFGHIJKLMNOPQRSTUVWX0123456789 !$%,.'

function Convert-Text([string]$s) {
    $actions = New-Object System.Collections.ArrayList
    $text = ''
    $curMod = ''
    $modKc = @{ 'S' = 'KC_LSFT'; 'A' = 'KC_RALT' }
    function Flush-Text { if ($text.Length -gt 0) { [void]$actions.Add(@('text', $text)); Set-Variable -Scope 1 text '' } }
    function Set-Mod([string]$m) {
        if ($curMod -ne $m) {
            if ($curMod -ne '') { [void]$actions.Add(@('up', $modKc[$curMod])) }
            if ($m -ne '') { [void]$actions.Add(@('down', $modKc[$m])) }
            Set-Variable -Scope 1 curMod $m
        }
    }
    for ($i = 0; $i -lt $s.Length; $i++) {
        $c = [string]$s[$i]
        if ($TextSafe.IndexOf($c, [StringComparison]::Ordinal) -ge 0) {
            Set-Mod ''
            $text += $c
            continue
        }
        if (-not $DE.ContainsKey($c)) {
            throw ("Zeichen an Position {0} wird nicht unterstuetzt (nicht auf der deutschen T1-Tastatur ohne Unicode-Eingabe)." -f ($i + 1))
        }
        Flush-Text
        $kc, $m, $dead = $DE[$c]
        Set-Mod $m
        [void]$actions.Add(@('tap', $kc))
        if ($dead) { Set-Mod ''; [void]$actions.Add(@('tap', 'KC_SPC')) }
    }
    Flush-Text
    Set-Mod ''
    # Speicherbedarf wie im Vial-Protokoll >= 2: Text 1 Byte/Zeichen (UTF-8, hier nur ASCII),
    # Tap/Down/Up mit Basis-Keycode 3 Byte, plus 1 Byte Null-Terminator je Makro.
    $bytes = 1
    foreach ($a in $actions) { if ($a[0] -eq 'text') { $bytes += $a[1].Length } else { $bytes += 3 } }
    $json = '[' + (($actions | ForEach-Object { '[' + (($_ | ForEach-Object { ConvertTo-Json -Compress -InputObject ([string]$_) }) -join ', ') + ']' }) -join ', ') + ']'
    return @{ Json = $json; Bytes = $bytes; Actions = $actions.Count }
}

if ($SelfTest) {
    $dummies = @(
        ('Aa1!Zz@' + [char]0x20AC + '{[]}\~|^`' + [char]0x00B4),
        ('yYzZ' + $ae + $oe + $ue + $sz + $ae.ToUpperInvariant() + $oe.ToUpperInvariant() + $ue.ToUpperInvariant() + [char]0x00A7 + [char]0x00B0 + [char]0x00B5 + [char]0x00B2 + [char]0x00B3),
        'Dummy-Test_123:#<>?=*+''";&/()',
        'hallo welt.'
    )
    foreach ($d in $dummies) {
        $r = Convert-Text $d
        "{0} Aktionen, {1} Byte" -f $r.Actions, $r.Bytes
        $r.Json
    }
    return
}

function Read-Plain([string]$prompt) {
    $sec = Read-Host -AsSecureString -Prompt $prompt
    $ptr = [Runtime.InteropServices.Marshal]::SecureStringToBSTR($sec)
    try { return [Runtime.InteropServices.Marshal]::PtrToStringBSTR($ptr) }
    finally { [Runtime.InteropServices.Marshal]::ZeroFreeBSTR($ptr) }
}

$p1 = Read-Plain 'Text fuer das Makro (verdeckt)'
$p2 = Read-Plain 'Zur Kontrolle noch einmal'
if ($p1 -cne $p2) { $p1 = $null; $p2 = $null; Write-Host 'Eingaben stimmen nicht ueberein. Abbruch.'; exit 1 }
$p2 = $null
try { $r = Convert-Text $p1 } finally { $p1 = $null }

# In die Zwischenablage, ausgeschlossen vom Verlauf (Win+V) und von der Cloud-Synchronisation.
Add-Type -AssemblyName System.Windows.Forms
$zero = [byte[]](0, 0, 0, 0)
$data = New-Object System.Windows.Forms.DataObject
$data.SetData([System.Windows.Forms.DataFormats]::UnicodeText, $r.Json)
$data.SetData('ExcludeClipboardContentFromMonitorProcessing', (New-Object System.IO.MemoryStream (, $zero)))
$data.SetData('CanIncludeInClipboardHistory', (New-Object System.IO.MemoryStream (, $zero)))
$data.SetData('CanUploadToCloudClipboard', (New-Object System.IO.MemoryStream (, $zero)))
[System.Windows.Forms.Clipboard]::SetDataObject($data, $true)
$r.Json = $null

Write-Host ("Makro in der Zwischenablage: {0} Aktionen, {1} Byte Makrospeicher." -f $r.Actions, $r.Bytes)
Write-Host 'Vial: Makro-Tab -> "Open Text Editor..." -> alles markieren -> Strg+V -> Apply -> Save.'
Read-Host 'Danach Enter druecken, um die Zwischenablage zu leeren' | Out-Null
[System.Windows.Forms.Clipboard]::Clear()
Write-Host 'Zwischenablage geleert.'
