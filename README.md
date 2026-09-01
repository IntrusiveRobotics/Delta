# Intrusive Robotics Delta — Firmware

Open-source firmware for the **Intrusive Robotics Delta**, a compact high-power
GaN motor controller that is **compatible with VESC® software**.

This repository is the complete, buildable source for the firmware shipped on the
Delta. It is a fork of the VESC firmware project ([`vedderb/bldc`](https://github.com/vedderb/bldc))
and is licensed under the **GNU General Public License v3.0** (see [`LICENSE`](LICENSE)
and [`NOTICE.md`](NOTICE.md)). The upstream project's README is preserved as
[`README.upstream.md`](README.upstream.md).

> **Not affiliated with or endorsed by Benjamin Vedder or the VESC project.**
> "VESC" is a trademark of Benjamin Vedder. This product is *compatible with*
> VESC software; it is not a VESC-branded product.

---

## Firmware versioning

Delta firmware releases are labeled **`IR_FWx_y`** (for example `IR_FW1_5`,
`IR_FW2_2`). This is the Intrusive Robotics product version and is independent of
the upstream VESC firmware version it is built on (this release is based on VESC
firmware **6.05**). The product version is reported by the board as its hardware
name (`HW_NAME`), so VESC Tool will identify the connected ESC as e.g. `IR_FW1_5`.

Each release is published as a **git tag** and a **GitHub Release** carrying the
compiled binary. The tag's source tree is the *complete corresponding source* for
that release's binary, as required by the GPL.

| Release | VESC base | Status |
|---|---|---|
| `IR_FW3_11` | 6.05 | current |
| `IR_FW1_5` | 6.05 | superseded |

---

## Hardware target

- **MCU:** STM32F405RGT6, 168 MHz, 8 MHz HSE
- **Power stage:** 3× integrated GaN half-bridges (TTL PWM, integrated gate drivers)
- **Current sensing:** three-shunt, external current-sense amplifiers
- **Voltage range:** 6S–12S LiPo (22–50.4 V)
- **Control:** sensorless FOC

The board-specific configuration lives in
[`hwconf/intrusive_robotics/`](hwconf/intrusive_robotics/):
`hw_vesc_gan.h`, `hw_vesc_gan_core.h`, `hw_vesc_gan_core.c`.

---

## Building

Requires the `arm-none-eabi-gcc` toolchain (GCC 7.x, matching the upstream VESC
build) and GNU Make. Build the Delta target by name:

```bash
make fw_vesc_gan
```

Artifacts are written to `build/vesc_gan/` (`vesc_gan.bin`, `.hex`, `.elf`).

> **Note:** the Makefile does not tolerate spaces in the source path. If your
> checkout path contains spaces, build from a path without them.

---

## Flashing

Flash over SWD with an ST-Link and [`stlink`](https://github.com/stlink-org/stlink)
(`brew install stlink`):

```bash
# Verify the target is seen (expect chipid 0x413, STM32F4x5_F4x7)
st-info --probe

# Bootloader — once, on a fresh chip (generic VESC bootloader)
st-flash --reset write generic.bin 0x080E0000

# Main application
st-flash --reset write vesc_gan.bin 0x08000000
```

You can also flash the prebuilt `IR_FWx_y` binary from the matching GitHub Release.

### Flash layout (1 MB STM32F405RGT6)

| Region | Address | Contents |
|---|---|---|
| Vectors | `0x08000000` | application |
| Code | `0x0800C000` | application |
| Bootloader | `0x080E0000` | generic VESC bootloader |

---

## License

GPL v3.0. See [`LICENSE`](LICENSE) for the full text and [`NOTICE.md`](NOTICE.md)
for upstream provenance and trademark notices. You are free to use, study, modify,
and redistribute this firmware under the terms of the GPL.
