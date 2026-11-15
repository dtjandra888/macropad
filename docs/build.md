# Building

This repo has three things you can build:

| What | Where | Output |
|---|---|---|
| Firmware (includes the web UI) | repo root, CMake + Pico SDK | `build/macro.uf2` |
| Firmware unit tests | `tests/`, host CMake | `build-tests/test_*` |
| Web UI on its own (dev server, web tests) | `web/`, bun | `web/dist/` |

All commands below run from the repo root unless noted.

## Prerequisites

| Tool | Needed for | Notes |
|---|---|---|
| CMake ≥ 3.13 (≥ 3.14 for tests) | firmware, tests | |
| `arm-none-eabi-gcc` | firmware | ARM cross compiler |
| [Pico SDK](https://github.com/raspberrypi/pico-sdk) | firmware | developed against 2.3.0, with submodules (TinyUSB 0.18.0, lwIP 2.2.1) |
| [bun](https://bun.sh) | firmware, web UI | the firmware build also builds the web UI |
| host `gcc` (or `clang`) | tests | |
| git + network on the first test configure | tests | CMake downloads Unity v2.6.1 |

Pico SDK setup:

```bash
git clone https://github.com/raspberrypi/pico-sdk.git
cd pico-sdk && git submodule update --init   # TinyUSB and lwIP live in submodules
export PICO_SDK_PATH=/path/to/pico-sdk       # add to your shell profile
```

Install the web UI's dependencies once:

```bash
cd web && bun install
```

## Firmware

```bash
mkdir -p build && cd build
cmake ..
make            # produces build/macro.uf2
```

What the build does:

- Targets the `waveshare_rp2040_zero` board (set in the root `CMakeLists.txt`).
- Runs `bun run build` and `bun run generate-fsdata` in `web/` whenever web sources change. This writes `build/lib/httpd/fsdata.c`, which embeds the web UI in the firmware. That file is generated and not committed.
- Exports `build/compile_commands.json` for clangd.

### Flashing

1. Hold the RP2040-Zero's BOOT button while plugging it in. It mounts as a USB drive.
2. Copy `build/macro.uf2` onto that drive.

On Linux, `./scripts/transfer.sh` does step 2 for you. It assumes the drive is `/dev/sda1` and mounts it at `/mnt/usb` with `sudo`. Check `lsblk` first if you have other USB drives connected.

### Running config mode

Hold the config button while plugging the macropad in. The LED turns blue, and the device shows up as a USB network adapter with the web UI at http://192.168.7.1. If Linux doesn't bring the interface up on its own:

```bash
sudo ip addr add 192.168.7.2/24 dev <enx*>
sudo ip link set <enx*> up
```

## Firmware unit tests

The C logic (JSON parsing, config storage, key debounce, macro playback, HID report merging) is tested on your machine. No hardware is needed.

```bash
cmake -S tests -B build-tests
cmake --build build-tests
ctest --test-dir build-tests --output-on-failure
```

Run a single suite directly to see each test's result:

```bash
./build-tests/test_json_parser
```

How it's put together:

- `tests/` is its own CMake project. The firmware build doesn't include it, and it doesn't need the Pico SDK.
- Tests use [Unity](https://github.com/ThrowTheSwitch/Unity). CMake downloads it on the first configure.
- Builds use AddressSanitizer and UndefinedBehaviorSanitizer, so out-of-bounds reads and writes fail the test.
- `tests/stubs/` holds small stand-ins for the Pico SDK, TinyUSB and lwIP headers, with only what our code uses.
- `tests/fakes/` holds hand-written fakes (clock, GPIO, flash, USB HID, pbufs) that tests can control and inspect.
- Each `tests/test_<name>.c` links one real firmware source file plus the fakes it needs.

Not tested: the libraries themselves, USB descriptors and callbacks, RNDIS, `main.c`, and the OLED/LED drivers.

### Adding a test suite

1. Create `tests/test_<name>.c` with `setUp`, `tearDown`, the tests, and a `main` that calls `RUN_TEST` for each one.
2. Register it in `tests/CMakeLists.txt` with the source under test and the fakes it needs:

   ```cmake
   add_unit_test(test_<name> ${ROOT}/<source>.c fakes/fake_time.c)
   ```

3. Re-run the three commands above.

Firmware modules keep state in `static` variables. Reset that state in `setUp` through the module's public functions; for example, `test_hid_report.c` releases every key and sends an empty report.

## Web UI

These run in `web/`. You only need them to work on the UI itself, since the firmware build already builds the UI.

```bash
bun run dev            # vite dev server + mock API (web/server.ts on port 3000, proxied at /api)
bun run check          # svelte-check + tsc
bun run test:run       # vitest, all tests once
bun run build:device   # vite build + web/fs/fsdata.c (for inspection only)
```

The mock server reads and writes `web/config.json`. That file uses an older format than the firmware: `modifier` is a number and key names have no `HID_` prefix.

## Clean builds

```bash
rm -rf build build-tests
```
