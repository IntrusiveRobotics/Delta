# Intrusive Robotics Delta — Firmware Changelog

Product-level changelog for **Delta** firmware (`IR_FWx_y`). This is separate from
[`CHANGELOG.md`](CHANGELOG.md), which is the upstream VESC firmware changelog.

All releases are built on VESC firmware **6.05**, target `fw_vesc_gan`, MCU
STM32F405RGT6. Board configuration lives in `hwconf/intrusive_robotics/`.

---

## IR_FW3_11 — 2026-08-31

Rolls up every configuration change made since `IR_FW1_5`. All changes are in
`hwconf/intrusive_robotics/hw_vesc_gan_core.h`; no changes to shared VESC code.

### Hardware

- **Board rev 2 voltage divider.** Top leg `VIN_R1` 43.2 k → **68.1 k**. The divider
  is shared by the VBAT sense and all three phase-voltage senses (four identical
  dividers: R3/R4/R7/R15 top, R2/R5/R8/R16 bottom), so all four scale together and
  `ADC_VOLTS_PH_FACTOR` stays at 1.0. Full-scale sense range 68.1 V → 105.4 V; the
  93 V `HW_LIM_VIN` remains the binding limit.
  **This firmware requires a rev 2 board.** Running it on a rev 1 board (43.2 k)
  will misreport input and phase voltages.

- **Board NTC beta corrected to 4067 K** (was 3380 K). Confirmed by
  bench measurement on 2026-08-31. Note that neither the intended part
  (NTCG164BH103FT1S, datasheet beta 3380 K) nor the schematic's NTCG103JF103FT1
  (3435 K) matches the measured value — the populated part is evidently neither.
  Reconcile the BOM against the real part before respinning, and re-measure if the
  thermistor is ever substituted.

### Firmware

- **FOC switching frequency 60 kHz → 30 kHz** (`MCCONF_FOC_F_ZV`). Fixes an
  independent-watchdog reset during the FOC detection "apply settings" step: at
  60 kHz the FOC ISR overran its budget and starved the watchdog kicker. Diagnosed
  via `RCC_CSR` reset-flag analysis.

- **FET thermal derating raised** — `MCCONF_L_LIM_TEMP_FET_START` 85 → **95 °C**,
  `MCCONF_L_LIM_TEMP_FET_END` 100 → **115 °C**. Referenced to the board NTC and
  still under the 160 °C `HW_LIM_TEMP_FET` clamp. These limits are only meaningful
  because the 4067 K beta above is correct.

- **Phase filters default to OFF** (`MCCONF_FOC_PHASE_FILTER_ENABLE false`). The
  hardware is unchanged and the filters remain toggleable in VESC Tool.

- **Phase-filter max ERPM 4000 → 500** (`MCCONF_FOC_PHASE_FILTER_MAX_ERPM`).

- **Motor temperature sensor defaults to disabled**
  (`MCCONF_M_MOTOR_TEMP_SENS_TYPE TEMP_SENSOR_DISABLED`). No motor thermistor is
  fitted; a floating input could otherwise fake a hot reading. Re-selectable in
  VESC Tool.

- `HW_NAME` → `IR_FW3_11`.

### Dead time

- **`HW_DEAD_TIME_NSEC` 60.0 → 40.0.**

  Note that `conf_general_calculate_deadtime()` quantises this to the timer's
  dead-time generator step: at 168 MHz the DTG timebase is 5.952 ns, so 40.0
  resolves to **DTG=6 → 35.7 ns actual**. The LMG2100R026 has no body diode, so any
  dead-time conduction flows through the GaN reverse channel at ~2–3 V drop. If you
  are commissioning a new board, scope the switch nodes for shoot-through.

### Unchanged from IR_FW1_5

`MCCONF_FOC_SAMPLE_V0_V7 false` (ISR at f_zv/2), `MCCONF_L_IN_CURRENT_MAX/MIN`
±60 A, `MCCONF_L_MAX_ABS_CURRENT` 80 A, `HW_LIM_VIN` 8–93 V.

---

## IR_FW1_5 — 2026-06-18

Initial public release. VESC firmware 6.05, `HW_NAME` = `IR_FW1_5`, 60 ns nominal dead time, 43.2 k divider (board rev 1), NTC beta 3380 K.
