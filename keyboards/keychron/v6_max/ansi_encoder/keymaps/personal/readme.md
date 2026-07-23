# Keychron V6 Max personal keymap

This is an RGB-free ANSI keymap for the Keychron V6 Max with an encoder. It
prioritizes accessibility, wireless battery life, safe reset actions, VIA
support, and compatibility with the stock Keychron EEPROM layout.

## Physical layout changes

Both positions of the operating-system switch use the same Windows-style
layout.

- The physical Right Windows key is Fn.
- The physical Fn key is Application/Menu.
- The Cortana key is Sleep.
- The stock `UG_NEXT` position is Pause.
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
