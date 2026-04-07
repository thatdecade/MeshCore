# nRF52 Power Management

## Overview

The nRF52 power management module protects batteries from over-discharge, reduces the chance of brownout-related flash corruption, and supports automatic recovery after a low-voltage shutdown.

When enabled and configured, the firmware:

- reads battery voltage early in boot
- skips the boot lockout check when external power is present
- enters protective `SYSTEMOFF` if battery voltage is below the configured threshold
- records the shutdown reason in `GPREGRET2`
- wakes again when battery voltage recovers through LPCOMP or when external power is connected through VBUS, depending on board support

## Accessing Power Management Commands

### Normal CLI

The `get pwrmgt.*` and `set pwrmgt.*` commands are part of the normal MeshCore CLI.

These commands work only on firmware and interfaces that expose the text CLI.

### Companion firmware note

On companion firmware, the BLE UART and serial companion interfaces use the MeshCore companion protocol, not a plain text shell. A serial terminal at `115200` will usually **not** respond to commands like `get pwrmgt.bootmv` unless you are using a client that speaks the companion protocol.

### CLI Rescue mode

Companion firmware still supports **CLI Rescue** during early boot. This is a small recovery shell intended for rescue and filesystem maintenance.

CLI Rescue is **not** the full CommonCLI, so `get pwrmgt.*` and `set pwrmgt.*` are not available there.

Use CLI Rescue for commands like:

- `ls /`
- `ls UserData/`
- `ls ExtraFS/`
- `erase`
- `rebuild`
- `set pin 123456`

If BLE is still connected while you are in CLI Rescue, you may see log spam such as:

```
BLE: onBleUartRX: recv queue full, dropping data
```

That means a BLE client is still sending data while the rescue shell is active. Disconnect the phone app or other BLE client before using rescue mode over serial.

## CLI Commands

| Command | Description |
|---------|-------------|
| `get pwrmgt.support` | Returns `supported` or `unsupported` |
| `get pwrmgt.source` | Returns current power source: `battery` or `external` |
| `get pwrmgt.bootreason` | Returns reset reason and shutdown reason strings |
| `get pwrmgt.bootmv` | Returns battery voltage at boot in millivolts |
| `get pwrmgt.batt` | Returns configured battery chemistry: `liion`, `lfp`, or `lto` |
| `set pwrmgt.batt liion\|lfp\|lto` | Sets battery chemistry, persisted and applied on next boot |
| `get pwrmgt.bootlock` | Returns boot protection state: `enabled` or `disabled` |
| `set pwrmgt.bootlock on\|off` | Enables or disables boot protection, persisted and applied after reboot |
| `poweroff` / `shutdown` | Immediate power off. This does not configure voltage wake, so recovery requires VBUS or another board-specific wake source. |

On boards without power management support, all commands except `get pwrmgt.support` return `ERROR: Power management not supported`.

## Configuration

### Battery chemistry

Set the correct chemistry so the firmware uses the correct boot lockout threshold.

| Chemistry | CLI value | Boot lockout threshold |
|-----------|-----------|------------------------|
| Li-ion / LiPo | `liion` | 3000 mV |
| Lithium Iron Phosphate | `lfp` | 2500 mV |
| Lithium Titanate Oxide | `lto` | 1800 mV |

### Boot protection

Boot protection is disabled by default in Stage 1 v2.1.

Enable it with:

```
set pwrmgt.bootlock on
```

Then reboot and verify with:

```
get pwrmgt.batt
get pwrmgt.bootlock
get pwrmgt.bootmv
get pwrmgt.bootreason
```

## Supported Boards in This Branch

This table reflects the state of this fork branch, including the ThinkNode M1 port.

| Board | Implemented | LPCOMP wake | VBUS wake | Notes |
|-------|-------------|-------------|-----------|-------|
| Seeed Studio XIAO nRF52840 (`xiao_nrf52`) | Yes | Yes | Yes | Per-chemistry LPCOMP thresholds |
| RAK4631 (`rak4631`) | Yes | Yes | Yes | Per-chemistry LPCOMP thresholds |
| Heltec T114 (`heltec_t114`) | Yes | Yes | Yes | Per-chemistry LPCOMP thresholds |
| SenseCAP Solar (`sensecap_solar`) | Yes | Yes | Yes | Existing power management implementation |
| ThinkNode M1 (`thinknode_m1`) | Yes | Partial | Yes | Li-ion LPCOMP wake is configured. LFP and LTO currently use VBUS-only wake until hardware calibration is completed. |
| ThinkNode M3 (`thinknode_m3`) | No | No | No | Not ported in this branch |
| ThinkNode M6 (`thinknode_m6`) | No | No | No | Not ported to the Stage 1 v2.1 per-chemistry model in this branch |
| Promicro nRF52840 | No | No | No | Not implemented here |
| RAK WisMesh Tag | No | No | No | Not implemented here |
| Heltec Mesh Solar | No | No | No | Not implemented here |
| LilyGo T-Echo / T-Echo Lite | No | No | No | Not implemented here |
| WIO Tracker L1 / L1 E-Ink | No | No | No | Not implemented here |
| WIO WM1110 | No | No | No | Not implemented here |
| Mesh Pocket | No | No | No | Not implemented here |
| Nano G2 Ultra | No | No | No | Not implemented here |
| T1000-E | No | No | No | Not implemented here |
| Ikoka Nano / Stick / Handheld (nRF) | No | No | No | Not implemented here |
| Keepteen LT1 | No | No | No | Not implemented here |
| Minewsemi ME25LS01 | No | No | No | Not implemented here |

## How It Works

### Boot voltage protection

On boot, the firmware:

1. captures reset and shutdown reason state before later init clears those registers
2. reads stored power preferences early from `InternalFS` so boot lockout can use persisted settings
3. reads battery voltage
4. skips the lockout check if external power is present
5. compares the voltage against the boot lockout threshold for the configured battery chemistry
6. enters protective `SYSTEMOFF` if voltage is below threshold

On ThinkNode M1, the boot voltage check is performed **before** enabling the SX1262 radio power rail. This avoids adding radio load before the boot protection decision is made.

### Wake sources

A device in protective shutdown can be woken by one or both of these sources, depending on the board:

- **LPCOMP**: the nRF52 low power comparator monitors battery voltage during `SYSTEMOFF`. When battery voltage rises above the configured threshold, it wakes the system.
- **VBUS**: USB or other external power on VBUS wakes the device through the POWER peripheral.

If a board uses `0xFF` for a chemistry-specific LPCOMP REFSEL value, that chemistry uses **VBUS-only wake** for low-voltage recovery.

### Shutdown reasons

The firmware records why the device entered `SYSTEMOFF`. This survives through `SYSTEMOFF` and can be read after the next boot with `get pwrmgt.bootreason`.

| Code | Name | Description |
|------|------|-------------|
| `0x00` | `NONE` | Normal boot or no previous shutdown |
| `0x4C` | `LOW_VOLTAGE` | Runtime low voltage threshold reached |
| `0x55` | `USER` | Manual shutdown via `poweroff` or `shutdown` |
| `0x42` | `BOOT_PROTECT` | Boot voltage protection triggered |

### Boot reason tracking

The firmware captures `RESETREAS` and `GPREGRET2` very early in boot before later system initialisation clears them. This allows `get pwrmgt.bootreason` to report both the wake source and the previous shutdown reason.

## LPCOMP Wake Voltage Reference

The LPCOMP wake voltage depends on the board's voltage divider ratio and the configured `REFSEL` value.

**Wake voltage formula**

```
VBAT_wake = REFSEL_fraction x VDD_sys x ADC_MULTIPLIER
```

Where:

- `VDD_sys` is typically about 3.0 to 3.3 V during `SYSTEMOFF`
- `ADC_MULTIPLIER` is the board's voltage divider scale factor

**REFSEL fractions**

| REFSEL | Fraction | REFSEL | Fraction |
|--------|----------|--------|----------|
| 0 | 1/8 | 8 | 1/16 |
| 1 | 2/8 | 9 | 3/16 |
| 2 | 3/8 | 10 | 5/16 |
| 3 | 4/8 | 11 | 7/16 |
| 4 | 5/8 | 12 | 9/16 |
| 5 | 6/8 | 13 | 11/16 |
| 6 | 7/8 | 14 | 13/16 |
| 7 | ARef | 15 | 15/16 |

**Per-board wake configuration examples**

| Board | ADC_MUL | Li-ion REFSEL | Li-ion wake range | LFP REFSEL | LFP wake range | LTO REFSEL | LTO wake range |
|-------|---------|---------------|-------------------|------------|----------------|------------|----------------|
| RAK4631 | about 1.73 | 4 (5/8) | about 3.24 to 3.57 V | 4 (5/8) | about 3.24 to 3.57 V | 11 (7/16) | about 2.27 to 2.50 V |
| Heltec T114 | 4.90 | 1 (2/8) | about 3.68 to 4.04 V | 9 (3/16) | about 2.76 to 3.03 V | 0 (1/8) | about 1.84 to 2.02 V |
| XIAO nRF52 | 3.0 | 2 (3/8) | about 3.38 to 3.71 V | 10 (5/16) | about 2.81 to 3.09 V | 1 (2/8) | about 2.25 to 2.47 V |
| ThinkNode M1 | 2.0 | 3 (4/8) | about 3.00 to 3.30 V | `0xFF` | VBUS-only wake | `0xFF` | VBUS-only wake |

For ThinkNode M1, Li-ion is the validated starting path. LFP and LTO boot thresholds are defined, but automatic battery-voltage wake for those chemistries is intentionally disabled until board-specific calibration is completed.

## Testing Notes

For companion firmware builds, power-management validation is often easier to do by observed behavior rather than text CLI access.

Recommended checks:

- repeated cold boot on USB
- repeated cold boot on battery only
- confirm no boot loops on a healthy battery
- confirm clean recovery when VBUS is attached after a low-voltage state
- confirm radio still initialises after boot on ThinkNode M1, since radio power is enabled after the boot voltage check

## Debug Output

When built with `MESH_DEBUG=1`, the power management module logs at boot with lines like:

```
PWRMGT: Reset = Wake from LPCOMP (0x20000); Shutdown = Low Voltage (0x4C)
PWRMGT: Boot protection enabled (Li-ion), threshold=3000 mV
PWRMGT: Boot voltage=3450 mV
PWRMGT: LPCOMP wake configured (AIN7, ref=3/8 VDD)
PWRMGT: VBUS wake configured
```

If boot protection is disabled, you should instead see:

```
PWRMGT: Boot protection disabled
```

## References

- [nRF52840 Product Specification - POWER](https://infocenter.nordicsemi.com/topic/ps_nrf52840/power.html)
- [nRF52840 Product Specification - LPCOMP](https://infocenter.nordicsemi.com/topic/ps_nrf52840/lpcomp.html)
- [SoftDevice S140 API - Power Management](https://infocenter.nordicsemi.com/topic/sdk_nrf5_v17.1.0/group__nrf__sdm__api.html)
