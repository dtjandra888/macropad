# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project

RP2040-Zero macropad: C firmware (Pico SDK + TinyUSB + lwIP) plus a Svelte/TypeScript web UI that is compiled into the firmware and served by the device itself for editing macros.

## Commands

### Firmware
Requires `PICO_SDK_PATH` set and `bun` on PATH (the firmware build also builds the web UI).

```bash
mkdir -p build && cd build && cmake .. && make   # produces build/macro.uf2
./scripts/transfer.sh                            # from repo root: copies build/macro.uf2 to the RP2040 mounted at /dev/sda1 (BOOTSEL mode)
```

There are no firmware tests. `compile_commands.json` is exported for clangd (`.clangd` adds the arm-none-eabi include path).

### Web UI (run in `web/`)
```bash
bun install
bun run dev            # vite dev server + mock API (server.ts, port 3000, proxied at /api)
bun run check          # svelte-check + tsc
bun run test:run       # vitest, all tests once
bunx vitest run tests/config.test.ts   # single test file
bun run build:device   # vite build + regenerate fsdata.c (CMake does this automatically)
```

The mock server (`web/server.ts`) reads/writes `web/config.json`, which uses an older format (`modifier` as a number, keys without the `HID_` prefix) than the firmware's current JSON (`"key": "HID_C"`, `"modifiers": ["HID_MOD_LCTRL"]`).

## Architecture

### Two boot modes (`main.c`)
At boot, the config button (`CONFIG_BTN_PIN`) is sampled once:
- **Keyboard mode** (default): 1 ms loop of `key_scan()` → `process_key_events()` → `macro_task()`. Key index `r * KEY_COLS + c` maps directly to `config.macros[index]`; `macro.c` replays strokes as USB HID reports.
- **Config mode** (button held, LED turns blue): starts a USB network interface (`lib/rndis`) with an lwIP HTTP server at 192.168.7.1. Never returns to keyboard mode without a reset.

The USB device is a composite (HID + network). `usb_descriptors.c` exposes two configurations — RNDIS (Windows/Linux) and CDC-ECM (macOS/Linux) — Windows 7+ binds RNDIS via the class codes in `TUD_RNDIS_DESCRIPTOR` (0xE0/0x01/0x03); there are no Microsoft OS descriptors. Linux picks the ECM configuration, Windows always uses RNDIS (config 1), so RNDIS bugs only show on Windows. The RNDIS/ECM macros each create two interfaces (control + data), so `ITF_NUM_TOTAL` must count both — Windows fails the whole device with Code 10 if `bNumInterfaces` is wrong.

### Config data flow
- `include/config.h`: `Config` (fixed `macros[3]`, `MACRO_MAX_STROKES` = 5 in `macro.h`) wrapped in `ConfigStorage` with a magic + CRC32 header, stored in the **last flash sector** (`config.c`).
- On boot, `check_config_flash()` loads it; if invalid, it writes `default_config` from `config/key_config.h`.
- `lib/httpd/config_api.c` implements `GET /api/config` (via `fs_open_custom`, serializes to JSON) and `POST /api/config` (lwIP `httpd_post_*` callbacks, hand-rolled parser in `json_parser.c`, then `save_config`).
- `hid_json.c` maps HID keycodes/modifiers ↔ the `HID_*` / `HID_MOD_*` strings used in JSON. Adding a supported key means updating this table (and the web UI's types/validation in `web/src/types.ts` / `web/src/lib/config.ts`).
- The web UI validates config client-side (`isValidConfig`); the firmware re-validates during parsing.

### Hardware/board config
- `config/macropad_config.h`: key matrix size and pins, OLED SPI pins, WS2812 pin, config button pin.
- `config/key_config.h`: default macros written on first boot.
- `PICO_BOARD` is `waveshare_rp2040_zero` (set in root `CMakeLists.txt`).
- `logger.c` logs to the OLED (`log_info`/`log_error`), not to UART/USB stdio (both disabled).

### Web UI embedding
`lib/httpd/CMakeLists.txt` runs `bun run build` and `bun run generate-fsdata` (`web/scripts/generate-fsdata.ts`) whenever web sources change, writing `lib/httpd/fsdata.c` **into the source tree**. That generated file is committed, so expect it in diffs after web changes. Vite uses `base: "./"` so assets resolve from the device's filesystem.

### Host networking (Linux)
If the device's web server is unreachable in config mode, bring up the interface manually (from `lib/httpd/notes-for-when-you-forget.md`):
```bash
sudo ip addr add 192.168.7.2/24 dev <enx*>
sudo ip link set <enx*> up
```
