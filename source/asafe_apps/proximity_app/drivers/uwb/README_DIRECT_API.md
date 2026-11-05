# DW3000 direct API (no probe interface)

This tree has been reduced to a **DW3000-only** driver whose functions are reached by
name instead of through the `dwt_probe()` function-pointer tables. The point is code
size: with the vendor layout the linker had to keep the whole driver.

## Why the old layout kept everything

`dwt_probe()` stored `&dw3000_driver` in the chip context. `dw3000_driver` points at
`dwt_ops_s` / `dwt_mcps_ops_s`, whose `.ioctl` member is `dwt_ioctl()` — a ~1150-line
switch that names *every* `ull_*()` function in `dw3000_device.c`. Calling any single
API, say `dwt_configure()`, reached `ioctl()`, so the linker had to keep the switch, so
it had to keep the entire driver.

## What changed

| | Before | After |
|---|---|---|
| Init | `dwt_probe(&dw3000_probe_interf)` | `dwt_attach(&dw3000_spi_fct, port_wakeup_dw1000)` |
| Dispatch | `dw->dwt_driver->dwt_ops->ioctl(dw, DWT_CONFIGURESLEEP, 0, &tmp)` | `dw3000_ioc_configuresleep(dw, 0, &tmp)` |
| Chip selection | runtime, from a driver list | compile time |

* **`dw3000/dw3000_direct.h` / `dw3000/dw3000_direct.inc`** (new, generated) — one
  externally visible function per former `ioctl()` command, e.g. `DWT_STARTTX` becomes
  `dw3000_ioc_starttx()`. Each body is the **verbatim** body of the matching `case`,
  wrapped in `do { ... } while (false)` so `break` keeps its original meaning. The
  `.inc` is `#include`d at the end of `dw3000_device.c` because the `ull_*()` functions
  it calls have internal linkage.
* **`dw3000/dw3000_device.c`** — 16 functions that used to be reached through the ops
  tables (`ull_configure`, `ull_initialise`, `ull_rxenable`, …) lost their `static`.
  `dwt_ioctl()`, the MCPS glue and the three descriptor tables are now behind
  `#ifdef DW3000_LEGACY_PROBE_API` (off by default). The direct entry points are always
  built, so defining that macro gives you both.
* **`deca_compat.c`** — the public `dwt_xxx()` API is unchanged. Its 175 `ioctl()` call
  sites now call the direct entry points. `dwt_attach()` was added; `dwt_probe()` is kept
  as a thin wrapper so existing application code still builds.
* **`probe_interface/`** — no longer references `dw3000_driver` (that reference alone
  would pull the tables, and therefore the whole driver, back in).

## Behaviour notes

21 API calls are implemented only by the DW3720 (dual-SPI semaphores, the timers,
WiFi-coex GPIO, ADC capture, `dwt_xtal_temperature_compensation`, …). On a DW3000 the
old `ioctl()` answered these from its `default:` case with `-1`; they now call
`dw3000_ioc_unsupported()`, which returns `-1`. Same result, no dispatcher.

Everything else is unchanged: same register writes, same ordering, same return values.

## Build

Required for the size win — without section GC the shims are still all emitted:

```
CFLAGS  += -ffunction-sections -fdata-sections
LDFLAGS += -Wl,--gc-sections
```

Sources to compile:

```
deca_compat.c
deca_rsl.c
qmath/qmath.c
dw3000/dw3000_device.c
probe_interface/deca_probe_interface.c
port/port_platform.c
```

No longer needed (drop them from the build):

* `dw3720/dw3720_device.c` — other chip.
* `deca_interface.c` — MCPS glue; only reachable through `dwt_mcps_ops_s`. Re-add it if
  you define `DW3000_LEGACY_PROBE_API`.

## Measured effect

Host build (x86-64, `-Os`), application calling `dwt_attach`, `dwt_initialise`,
`dwt_configure`, `dwt_set{rx,tx}antennadelay`, `dwt_writetxdata`, `dwt_writetxfctrl`,
`dwt_starttx`, `dwt_rxenable`, `dwt_readrxtimestamp`, `dwt_readdevid`:

```
.text   before: 37042 bytes
.text   after:  10735 bytes   (-26307, -71%)
```

Absolute numbers will differ on Cortex-M, but the effect is the same: you now pay for
the API you call.

## Regenerating

`dw3000_direct.h` / `dw3000_direct.inc` were generated from the `case` labels of the
original `dwt_ioctl()`. If you ever restore a vendor `dw3000_device.c`, re-derive them
rather than hand-editing.
