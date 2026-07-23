# Keychron V6 Max personal keymap

This is an RGB-free ANSI keymap for the Keychron V6 Max with an encoder. It
prioritizes accessibility, wireless battery life, safe reset actions, VIA
support, and compatibility with the stock Keychron EEPROM layout.

## Design decisions and maintenance invariants

This section explains choices that can otherwise look accidental. Read it
before simplifying the build rules, EEPROM layout, VIA initialization, or reset
sequence.

### Keep the RGB-free build local

Keychron adds proprietary RGB sources and definitions before keymap rules are
processed. Disabling `RGB_MATRIX_ENABLE` in `rules.mk` alone does not remove all
of those sources or their references.

The board-level `post_rules.mk` filters Keychron RGB sources, definitions, and
include paths only when `KEYMAP=personal`. This keeps unrelated keyboards and
the stock V6 Max keymap unchanged.

Two earlier approaches were rejected:

- A keymap-level `SRC :=` assignment forced QMK's recursively evaluated source
  list too early. Later Quantum, ChibiOS startup, and bootloader sources were
  omitted, causing linker failures.
- Reordering global libraries in `builddefs/build_keyboard.mk` hid that local
  Make error but could change static-library resolution for every QMK
  keyboard.

Do not reintroduce either approach. The global build rules should remain
upstream-compatible.

### Preserve the stock EEPROM layout

Stock Keychron RGB firmware reserves 518 bytes before wireless and VIA data.
Removing that block without compensation shifts every later address. Existing
wireless settings and VIA mappings would then be read from the wrong locations.

`KEYCHRON_RGB_EEPROM_COMPAT_SIZE=518` reserves the same span without compiling
RGB. The personal schema marker uses the final four bytes of that reserved
span. Do not shrink the reservation or move the marker without a deliberate
migration.

The schema version is the compatibility boundary:

- A matching marker means a routine personal-firmware update should preserve
  VIA mappings, wireless settings, and pairings.
- A missing or changed marker means the EEPROM format or reset policy changed,
  so first-install initialization runs.
- A schema change must update `PERSONAL_SCHEMA_MARKER`. Do not bump it for an
  ordinary code-only release because doing so intentionally triggers a full
  reset.

Returning to stock RGB firmware can overwrite the marker. This is intentional:
the next personal-firmware installation then receives a clean migration.

### Preserve VIA mappings without insecure telemetry

VIA normally uses the firmware build date as EEPROM magic. A new build date can
therefore reset otherwise compatible VIA mappings.

When the personal schema marker matches and VIA's stored magic bytes are intact,
`via_init_kb()` deliberately marks the current VIA date magic valid before QMK
performs its normal validity check. The schema marker is the stronger
compatibility decision for this keymap. Erased or partially erased VIA magic is
left invalid so QMK can initialize mappings, macros, and layout options safely.
Removing this migration as an apparent upstream-style cleanup would make
routine builds lose VIA mappings unless it is replaced with another strategy.

`VIA_INSECURE` is intentionally removed. VIA keymaps, macros, layout
options, and encoder remapping remain available, but unauthenticated live
matrix-row telemetry is not exposed over raw HID.

### Wait for the wireless module before factory reset

`keychron_common_init()` resets and initializes the LKBT51 wireless module
before `keyboard_post_init_user()` runs. Sending a factory-reset command
immediately afterward proved unreliable on hardware.

First-install reset is deferred for 3.5 seconds. This passes the LKBT51
three-second startup window and lets its wake sequence complete before the
command is sent. Do not shorten this to an immediate call without new hardware
evidence.

`P2P4G_CELAR_MASK` is `0x03`: bit 0 clears USB-A receiver pairing and bit 1
clears USB-C receiver pairing. Using only `0x01` or `0x02` leaves one receiver
type paired. USB-A clearing and re-pairing have been confirmed on hardware.

Fn+Period resets QMK, VIA, and wireless configuration but deliberately does not
send the module factory-reset command, so pairings survive. Fn+Slash and
first-install initialization call Keychron's full `factory_reset()` and then
rewrite only the schema marker.

Both manual reset paths cancel a pending first-install reset before changing
settings, preventing a delayed full reset from following a manual reset.

### Shared no-RGB guards are intentional

The shared Keychron changes are narrow compile-time guards, not unrelated
refactoring. They prevent RGB-only raw HID, animation, indicator, low-power,
and backlight calls from being compiled when neither LED Matrix nor RGB Matrix
exists. `wireless_get_host_index()` is exposed so the typed status report can
identify Bluetooth hosts without duplicating wireless state.

Do not replace missing RGB functions with empty global stubs. The guards keep
the feature model accurate and avoid affecting RGB-enabled keyboards.

### Status output is typed for accessibility

With RGB removed and no display available, Fn+I reports state by typing into the
focused application. It refuses to type while disconnected, during Bluetooth
PIN entry, or while any modifier is active. These checks reduce the chance of
injecting status text into a shortcut or command.

USB reports battery and voltage as unavailable because the wireless ADC values
are cached and can be misleading while USB is selected. Wireless reports use
the cached estimate and may be delivered gradually over Bluetooth.

### Destructive actions require holds

Bootloader entry and both reset actions use deferred callbacks so releasing a
key before its threshold cancels the action. Execution is intentionally silent
because no reliable non-RGB indicator exists. Do not replace the holds with
immediate keypress actions.

### Wireless shortcuts may override wireless mode

The connection switch chooses the transport at startup and whenever the switch
is moved. While either wireless transport is active, Fn+1 through Fn+3 can
switch from 2.4 GHz to Bluetooth, and Fn+4 can switch back to 2.4 GHz. The
override is temporary and is not written to EEPROM.

USB remains authoritative: wireless shortcuts do not change transport while the
connection switch is in the USB position. This prevents an accidental Fn chord
from disconnecting a wired session.

### Required checks after maintenance

Changes to shared Keychron guards, EEPROM offsets, build filtering, or reset
logic should compile both targets:

```bash
qmk compile -kb keychron/v6_max/ansi_encoder -km personal
```

```bash
qmk compile -kb keychron/v6_max/ansi_encoder -km keychron
```

Changes to schema, VIA initialization, or factory reset also require hardware
checks for pairing erasure, same-schema persistence, and status output.

## Physical layout changes

Both positions of the operating-system switch use the same Windows-style
layout.

- The physical Right Windows key is Fn.
- The physical Fn key is Application/Menu.
- The Cortana key is Sleep.
- The stock `UG_NEXT` position is Pause.
- The former F14 key is media Rewind.
- The former F15 key is media Fast Forward.
- The former F16 key is media Play/Pause.
- Turning the encoder changes volume on the base layers.
- The encoder does nothing while Fn is held.
- RGB controls and effects are disabled.

## Fn shortcuts

The Fn key is the physical key that was Right Windows in the stock layout.

- Fn+1 switches to Bluetooth and selects host 1. Hold it for two seconds to
  pair.
- Fn+2 switches to Bluetooth and selects host 2. Hold it for two seconds to
  pair.
- Fn+3 switches to Bluetooth and selects host 3. Hold it for two seconds to
  pair.
- Fn+4 switches to 2.4 GHz. Hold it for two seconds to pair a receiver.
- Fn+I types a connection and battery status report.
- Hold Fn+Comma for two seconds to enter the bootloader.
- Hold Fn+Period for three seconds to reset QMK, VIA, and wireless
  configuration while preserving Bluetooth and 2.4 GHz pairings.
- Hold Fn+Slash for three seconds to perform a full factory reset.

The function row provides brightness, task view, File Explorer, media, and
volume controls. Stock RGB-related Fn shortcuts are disabled.

The held bootloader and reset actions execute silently when their thresholds
are reached. Releasing either key early cancels the action.

## Status report

Fn+I types three lines into the currently focused application. Put the cursor
in a safe text field before using it.

USB output:

```text
Connection: USB
Battery: unavailable
Voltage estimate: unavailable
```

Wireless output follows this form:

```text
Connection: Bluetooth host 1
Battery: discharging, 75%
Voltage estimate: 3.92 V
```

The connection line reports Bluetooth host 1 through 3 or `2.4 GHz`. Battery
and voltage values are cached estimates. Wireless delivery may take a second or
two. No report is typed when the selected transport is disconnected, Bluetooth
PIN entry is active, or a real, weak, or one-shot modifier is active. The
report has no trailing newline.

## First installation and resets

The first boot of schema version 1 schedules a full reset 3.5 seconds after
startup so the wireless module is ready to receive the reset command. Keep USB
connected and wait at least 10 seconds before testing.

The automatic first-install reset and the Fn+Slash full reset clear:

- QMK settings and the complete emulated EEPROM data area.
- VIA keymaps, macros, and layout options.
- Keychron wireless configuration.
- Bluetooth host pairings 1 through 3.
- USB-A and USB-C 2.4 GHz receiver pairings.

The installed firmware and bootloader are not changed. A four-byte schema
marker is written after the reset so it does not repeat on every boot. The
external receiver firmware is not erased, but its keyboard association must be
re-established.

Routine updates using the same schema are designed to preserve VIA mappings,
wireless settings, and pairings. Returning to stock RGB firmware can overwrite
the marker, so a later return to this personal firmware will perform the
first-install reset again.

## Pairing

### Bluetooth

1. Set the connection switch to either wireless position.
2. Hold Fn+1, Fn+2, or Fn+3 for two seconds. The shortcut switches to
   Bluetooth if necessary.
3. Open Bluetooth settings on the host and select the keyboard.

Tapping Fn+1, Fn+2, or Fn+3 later selects or reconnects that host.

### USB-A or USB-C 2.4 GHz receiver

1. Unplug the receiver.
2. Set the connection switch to either wireless position.
3. Hold Fn+4 for two seconds, then release it.
4. Plug the receiver into the host.
5. Wait several seconds and test typing.

There is no visible pairing indicator because RGB support is disabled.

## VIA and security

VIA remains enabled for key remapping, macros, layout options, and encoder
remapping. `VIA_INSECURE` is removed, so unrestricted live matrix telemetry is
not available over raw HID.

The firmware reserves the stock 518-byte Keychron RGB EEPROM span even though
RGB is absent. This keeps wireless and VIA offsets compatible with stock
firmware.

## Wireless power saving

Connected wireless mode enters idle sleep after 600 seconds, or 10 minutes, of
inactivity.

## Build

Open the QMK MSYS terminal and change to the repository:

```bash
cd /d/projects/qmk_firmware
```

Compile the firmware:

```bash
qmk compile -kb keychron/v6_max/ansi_encoder -km personal
```

The output file is:

```text
D:\projects\qmk_firmware\keychron_v6_max_ansi_encoder_personal.bin
```

## Flash

From a running copy of this keymap, hold Fn+Comma for two seconds to enter the
bootloader. Then run:

```bash
qmk flash -kb keychron/v6_max/ansi_encoder -km personal
```

If the keyboard is already in bootloader mode, run the flash command directly.

## Hardware validation status

Confirmed on 2026-07-23:

- USB-A 2.4 GHz pairing erasure and re-pairing are confirmed on hardware.
- The USB-A receiver can be paired again with Fn+4.

Still to confirm:

- Bluetooth hosts 1 through 3 are cleared by a full reset.
- USB-C 2.4 GHz pairing is cleared by a full reset.
- VIA mappings and wireless pairings survive a routine same-schema update.
- Fn+I reports the expected values over USB, Bluetooth, and 2.4 GHz.
