# Integrated Humanitarian Coordination System (IHCS) — Internship

> ESP32-based firmware modules for a field-deployable humanitarian coordination terminal.
> Developed as part of the IHCS internship programme.

---

## Table of Contents

- [Overview](#overview)
- [Target Hardware](#target-hardware)
- [Project Structure](#project-structure)
- [Shared Components](#shared-components)
- [Phase 1 — Toolchain](#phase-1--toolchain)
- [Phase 2 — Hardware Bring-Up](#phase-2--hardware-bring-up)
- [Phase 3 — Integration](#phase-3--integration)
- [Phase 4 — Application Logic](#phase-4--application-logic)
- [Build & Flash](#build--flash)
- [Partition Tables](#partition-tables)
- [Google Drive Resources](#google-drive-resources)
- [License](#license)

---

## Overview

This repository contains a series of incremental ESP32 firmware projects built with the **ESP-IDF** (Espressif IoT Development Framework), organized by internship phase. Common hardware drivers (LCD, buttons, SPIFFS logging) live in shared, reusable ESP-IDF components under `components/`, rather than being duplicated across projects.

The work was carried out across four internship phases:

1. **Toolchain** — Getting the ESP-IDF toolchain working and flashing the first "hello world" LED blink.
2. **Hardware Bring-Up** — Individually testing UART serial output, GPIO button input, I2C LCD display, and SPIFFS persistent storage.
3. **Integration** — Combining all peripherals into a single interactive firmware built on shared components.
4. **Application Logic** — Building the actual state machine firmware for the field report terminal, on top of the shared components.

---

## Target Hardware

| Component | Details |
|---|---|
| **MCU** | ESP32-D0WD (dual-core Xtensa LX6) |
| **Framework** | ESP-IDF v6.0.1 (CMake-based build) |
| **LCD** | 16×2 character LCD via I2C (PCF8574 backpack, address `0x27`) |
| **Buttons** | 4× push-buttons, active-low with internal pull-up |
| **I2C Pins** | SDA = GPIO 21, SCL = GPIO 22 |
| **Button Pins** | SELECT = GPIO 32, UP = GPIO 33, DOWN = GPIO 25, BACK = GPIO 26 |

All shared pin definitions live in `components/include/board.h`.

---

## Project Structure

```
Integrated-Humanitarian-Coordination-System-ICHS-Internship/
├── README.md
├── components/                              # Shared ESP-IDF components
│   ├── include/                             # board.h — shared pin definitions
│   ├── lcd_i2c/                             # I2C 16x2 LCD driver
│   ├── buttons/                             # Debounced GPIO button handling
│   └── spiffs_log/                          # SPIFFS-backed logging
├── phase1_toolchain/
│   └── led_blink/                           # GPIO LED blink ("hello world")
├── phase2_bringup/
│   ├── uart_output/                         # UART serial heartbeat
│   ├── buttons_test/                        # Standalone button GPIO test
│   ├── lcd_test/                            # Standalone LCD driver test
│   └── spiffs_test/                         # Standalone SPIFFS test
├── phase3_integration/
│   └── combined_hardware_test/              # Integrated hardware test (all 3 components)
└── phase4_state_machine/
    └── report_terminal/                     # Field report terminal state machine firmware
```

Each project folder is an independent ESP-IDF project with its own `CMakeLists.txt`, `main/`, and `sdkconfig.defaults`.

---

## Shared Components

### `components/include` — `board.h`

Central pin definitions, shared by any component or project that needs them:

```c
#define LCD_SDA_PIN       GPIO_NUM_21
#define LCD_SCL_PIN       GPIO_NUM_22
#define BUTTON_SELECT_PIN GPIO_NUM_32
#define BUTTON_UP_PIN     GPIO_NUM_33
#define BUTTON_DOWN_PIN   GPIO_NUM_25
#define BUTTON_BACK_PIN   GPIO_NUM_26
```

### `components/lcd_i2c` — I2C LCD driver

4-bit I2C driver for a 16×2 character LCD with a PCF8574 backpack.

```c
void lcd_i2c_init(i2c_master_bus_handle_t bus);
void lcd_init(void);
void lcd_cmd(uint8_t cmd);
void lcd_data(uint8_t data);
void lcd_set_cursor(int row, int col);
void lcd_print(const char *str);
```

### `components/buttons` — Debounced GPIO buttons

Active-low, internal pull-up, 200 ms software debounce. Supports 4 buttons (SELECT, UP, DOWN, BACK).

```c
void buttons_init(void);
bool button_pressed(gpio_num_t pin);  // true once per press
```

### `components/spiffs_log` — SPIFFS logging

Mounts the `storage` SPIFFS partition and appends lines to a log file.

```c
esp_err_t spiffs_log_init(void);
void spiffs_log_write(const char *msg);
```

---

## Phase 1 — Toolchain

### `led_blink`

The simplest possible ESP-IDF application — configures an LED GPIO as output and blinks it every 500 ms. Used to verify the toolchain, flashing, and serial monitor were all working.

**Source:** `phase1_toolchain/led_blink/main/led_blink.c`

---

## Phase 2 — Hardware Bring-Up

### `uart_output` — Serial Heartbeat

Prints a boot message, then an incrementing "Heartbeat" counter every second over UART0.

**Source:** `phase2_bringup/uart_output/main/UART-output.c`

### `buttons_test` — GPIO Input with Debounce

Standalone test of the debounced button-reading approach later extracted into `components/buttons`.

**Source:** `phase2_bringup/buttons_test/main/Buttons-test.c`

### `lcd_test` — I2C LCD Driver

Standalone test of the LCD driver later extracted into `components/lcd_i2c`. Displays a boot message and an incrementing counter.

**Source:** `phase2_bringup/lcd_test/main/LCD-test.c`

### `spiffs_test` — Flash File System

Standalone test of SPIFFS mounting, writing, reading, and appending — the basis for `components/spiffs_log`. Uses a custom partition table (`partitions.csv`).

**Source:** `phase2_bringup/spiffs_test/main/spiffs_test.c`

---

## Phase 3 — Integration

### `combined_hardware_test`

Built on top of the three shared components (`lcd_i2c`, `buttons`, `spiffs_log`) rather than duplicating driver code.

**Behaviour:**

1. On boot, initializes SPIFFS, I2C, the LCD, and the buttons. LCD shows `"Ready"` / `"Logs: 0"`.
2. **Button 1 press:** Updates the LCD to show `"Button 1"` and the current log count.
3. **Button 2 press:** Increments the log counter, appends `"Log entry <n>"` to `/spiffs/log.txt` via `spiffs_log_write()`, and updates the LCD to show `"Logged!"`.
4. Main loop polls both buttons every 50 ms.

**Source:** `phase3_integration/combined_hardware_test/main/main.c`

---

## Phase 4 — Application Logic

### `report_terminal`

The state machine firmware for the actual field report terminal — the menu system a field worker interacts with. Built on the same shared components, with 2 extra buttons added (4 total: SELECT, UP, DOWN, BACK) to support real menu navigation.

Saving and syncing are placeholders at this stage — the focus was getting the full menu flow correct first.

For the full breakdown (screen-by-screen table, bugs found, UART trace, what's real vs placeholder), see the dedicated README:
📄 `phase4_state_machine/report_terminal/README.md`

**Source:** `phase4_state_machine/report_terminal/main/main.c`

---

## Build & Flash

Each project is an independent ESP-IDF project. To build and flash any of them:

```bash
# 1. Set up the ESP-IDF environment (if not already done)
. $IDF_PATH/export.sh

# 2. Navigate to the desired project
cd phase4_state_machine/report_terminal

# 3. Set the target chip (ESP32)
idf.py set-target esp32

# 4. Build
idf.py build

# 5. Flash to the device (adjust PORT as needed)
idf.py -p /dev/ttyUSB0 flash

# 6. Monitor serial output
idf.py -p /dev/ttyUSB0 monitor

# Or combine build + flash + monitor:
idf.py -p /dev/ttyUSB0 flash monitor
```

> **Note:** `lcd_test`, `spiffs_test`, `combined_hardware_test`, and `report_terminal` all reference the shared `components/` folder via `EXTRA_COMPONENT_DIRS` in their `CMakeLists.txt`. If you move a project to a different folder depth, that path must be updated accordingly.

---

## Partition Tables

`spiffs_test`, `combined_hardware_test`, and `report_terminal` use a custom partition table (`partitions.csv`):

| Name | Type | SubType | Offset | Size | Purpose |
|---|---|---|---|---|---|
| `nvs` | data | nvs | — | 0x6000 (24 KB) | Non-volatile storage (Wi-Fi config, etc.) |
| `phy_init` | data | phy | — | 0x1000 (4 KB) | PHY initialization data |
| `factory` | app | factory | — | 0x100000 (1 MB) | Application firmware |
| `storage` | data | spiffs | — | 0x60000 (384 KB) | SPIFFS file system |

---

## Google Drive Resources

- **Toolchain:** [Drive Folder](https://drive.google.com/drive/folders/1AljFMJDrY-QZh5AD0zJDmyHB4twc0G_r?usp=drive_link)
- **Hardware Bring-Up:** [Drive Folder](https://drive.google.com/drive/folders/1NcUKMDyomdr0Bg8sfuixllj89ywi88Lt?usp=drive_link)
- **Integration:** [Drive Folder](https://drive.google.com/drive/folders/1ZQKu4DGOcbPCFGfIWDvg8BQQo1RsRbDr?usp=drive_link)

---

## License

This project is developed as part of the IHCS internship programme. No explicit license is declared.
