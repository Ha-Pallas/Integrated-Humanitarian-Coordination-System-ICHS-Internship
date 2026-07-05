# Project Structure & Architecture Review

> An analysis of the folder structure, file organization, and code architecture patterns  
> adopted in the ICHS internship repository, with recommendations for improvement.  
> Written for the embedded dev intern — explains **what** to change, **how**, and **why**.

---

## Table of Contents

- [Current Structure Overview](#current-structure-overview)
- [What's Good About the Current Approach](#whats-good-about-the-current-approach)
- [Core Structural Issues](#core-structural-issues)
  - [1. Six Independent Projects Instead of One Project with Build Profiles](#1-six-independent-projects-instead-of-one-project-with-build-profiles)
  - [2. No `components/` Directory — Shared Code Is Copy-Pasted](#2-no-components-directory--shared-code-is-copy-pasted)
  - [3. No Header Files — Everything in a Single `.c` File](#3-no-header-files--everything-in-a-single-c-file)
  - [4. `sdkconfig` Duplicated 6 Times (~490 KB of Config Noise)](#4-sdkconfig-duplicated-6-times-490-kb-of-config-noise)
  - [5. No `.gitignore` — Build Artifacts in Version Control](#5-no-gitignore--build-artifacts-in-version-control)
  - [6. Flat Root Directory — No Logical Grouping by Phase](#6-flat-root-directory--no-logical-grouping-by-phase)
  - [7. Inconsistent Folder Naming — `kebab-case` vs `snake_case`](#7-inconsistent-folder-naming--kebab-case-vs-snake-case)
  - [8. The `Drives` File Has No Extension and Unclear Purpose](#8-the-drives-file-has-no-extension-and-unclear-purpose)
- [Variable Definition Patterns](#variable-definition-patterns)
  - [9. Global Variables Instead of Context Structs](#9-global-variables-instead-of-context-structs)
  - [10. Pin Definitions Scattered Per-File Instead of Centralized](#10-pin-definitions-scattered-per-file-instead-of-centralized)
  - [11. No `typedef` or Struct Grouping for Related Configuration](#11-no-typedef-or-struct-grouping-for-related-configuration)
  - [12. `static` Used Inconsistently](#12-static-used-inconsistently)
- [File Splitting Recommendations](#file-splitting-recommendations)
  - [13. The Combined Hardware Test Should Be Split Into Multiple Files](#13-the-combined-hardware-test-should-be-split-into-multiple-files)
  - [14. LCD Driver Should Be Its Own Component](#14-lcd-driver-should-be-its-own-component)
  - [15. Button Handling Should Be Its Own Component](#15-button-handling-should-be-its-own-component)
  - [16. SPIFFS Logging Should Be Its Own Component](#16-spiffs-logging-should-be-its-own-component)
- [Recommended Project Structure](#recommended-project-structure)
  - [Option A — Single Project with Build Targets (Recommended)](#option-a--single-project-with-build-targets-recommended)
  - [Option B — Multi-Project with Shared Components](#option-b--multi-project-with-shared-components)
- [Why This Matters for an Intern](#why-this-matters-for-an-intern)
- [Migration Checklist](#migration-checklist)

---

## Current Structure Overview

```
ICHS-Internship/
├── Drives                          # No extension, 3 lines of text
├── README.md
├── code_observations.md
├── led_blink/                      # Standalone ESP-IDF project
│   ├── CMakeLists.txt
│   ├── sdkconfig                   # ~82 KB
│   └── main/
│       ├── CMakeLists.txt
│       ├── led_blink.c             # 18 lines — all logic in one file
│       └── build/                  # Committed build artifacts!
├── UART-output/                    # Standalone ESP-IDF project
│   ├── CMakeLists.txt
│   ├── sdkconfig                   # ~82 KB
│   ├── sdkconfig.old               # ~82 KB — stale backup
│   └── main/
│       ├── CMakeLists.txt
│       └── UART-output.c           # 19 lines
├── Buttons-test/                   # Standalone ESP-IDF project
│   ├── CMakeLists.txt
│   ├── sdkconfig                   # ~82 KB
│   └── main/
│       ├── CMakeLists.txt
│       └── Buttons-test.c          # 62 lines
├── LCD-test/                       # Standalone ESP-IDF project
│   ├── CMakeLists.txt
│   ├── sdkconfig                   # ~82 KB
│   └── main/
│       ├── CMakeLists.txt
│       └── LCD-test.c              # 123 lines — LCD driver + app_main
├── spiffs_test/                    # Standalone ESP-IDF project
│   ├── CMakeLists.txt
│   ├── partitions.csv
│   ├── sdkconfig                   # ~82 KB
│   ├── sdkconfig.old               # ~82 KB
│   └── main/
│       ├── CMakeLists.txt
│       └── spiffs_test.c           # 54 lines
└── Combined_Hardware_Test/         # Standalone ESP-IDF project
    ├── CMakeLists.txt
    ├── partitions.csv
    ├── sdkconfig                   # ~82 KB
    ├── sdkconfig.old               # ~82 KB
    └── main/
        ├── CMakeLists.txt
        └── Combined_Hardware_Test.c  # 238 lines — LCD + buttons + SPIFFS + main
```

---

## What's Good About the Current Approach

Before diving into issues, it's worth acknowledging what was done right:

- **Incremental learning approach** — Each project builds on the previous one, testing one peripheral at a time before combining them. This is an excellent way to learn embedded development.
- **Standard ESP-IDF project layout** — Each folder follows the conventional `CMakeLists.txt` + `main/` structure that ESP-IDF expects. This shows the intern read the documentation.
- **Custom partition tables where needed** — `spiffs_test` and `Combined_Hardware_Test` correctly define `partitions.csv` files for SPIFFS storage.
- **The progression makes sense** — LED → UART → Buttons → LCD → SPIFFS → Combined. This is a logical order for hardware bring-up.

The issues below are about **how to evolve this from "internship learning exercises" to "maintainable project structure"**.

---

## Core Structural Issues

### 1. Six Independent Projects Instead of One Project with Build Profiles

**Current approach:** Each test is a completely separate ESP-IDF project with its own `CMakeLists.txt`, `sdkconfig`, and build directory.

**Why this is a problem:**

- **No code sharing** — The LCD driver in `LCD-test.c` is copy-pasted into `Combined_Hardware_Test.c`. If you fix a bug in one, you have to manually fix it in the other.
- **6 × `sdkconfig` = ~490 KB** of nearly identical configuration files. Any change to a common setting (e.g., flash size, CPU frequency) must be repeated 6 times.
- **6 × `CMakeLists.txt`** at the project level, all identical except the `project()` name.
- **No way to run all tests** — There's no unified build command. You have to `cd` into each directory and build separately.
- **Hard to navigate** — The root directory is cluttered with 6 peer-level folders with no hierarchy.

**Why it was probably done this way:** ESP-IDF's default project template creates a self-contained project folder. When you're learning, it's easiest to just copy the template each time. This is fine for learning but doesn't scale.

**Better approach — see [Recommended Project Structure](#recommended-project-structure) below.**

---

### 2. No `components/` Directory — Shared Code Is Copy-Pasted

**Current approach:** There is no `components/` directory. All code lives in `main/` of each project.

**Why this is a problem:**

ESP-IDF has a built-in component system that lets you write reusable modules and share them across projects. By not using it, the intern was forced to copy-paste the LCD driver (67 lines) from `LCD-test.c` into `Combined_Hardware_Test.c`. This means:

- Bug fixes must be applied in multiple places
- The two copies can drift out of sync over time
- There's no clear "API" for the LCD driver — it's just inline functions

**What a component looks like:**

```
components/
└── lcd_i2c/
    ├── CMakeLists.txt
    ├── include/
    │   └── lcd_i2c.h          # Public API
    └── lcd_i2c.c              # Implementation
```

```cmake
# components/lcd_i2c/CMakeLists.txt
idf_component_register(
    SRCS "lcd_i2c.c"
    INCLUDE_DIRS "include"
)
```

Then in any project that needs the LCD:
```cmake
# main/CMakeLists.txt
idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "."
    REQUIRES lcd_i2c
)
```

And in code:
```c
#include "lcd_i2c.h"  // Clean, documented API
```

**Why this matters:** Components are ESP-IDF's primary code reuse mechanism. Learning to use them is a fundamental skill for any ESP-IDF developer. The intern should practice this by extracting the LCD driver into a component.

---

### 3. No Header Files — Everything in a Single `.c` File

**Current approach:** Every project is a single `.c` file. No `.h` headers exist anywhere.

**Why this is a problem:**

- **No public API definition** — There's no clear separation between "functions that are part of the module's interface" and "internal helper functions." Everything is just in one file.
- **No way to reuse functions** — If `Buttons-test.c` wanted to use the LCD driver from `LCD-test.c`, there's no header to include. The only option is copy-paste.
- **Harder to navigate** — A 238-line file (`Combined_Hardware_Test.c`) with LCD driver + SPIFFS + buttons + main all mixed together is harder to read than 4 focused files.
- **No documentation of the interface** — A header file serves as documentation for what a module does. Without one, you have to read the entire `.c` file to understand the API.

**What a header file looks like:**

```c
// lcd_i2c.h — Public API for the I2C LCD driver
#pragma once

#include "esp_err.h"
#include "driver/gpio.h"
#include "driver/i2c_master.h"

typedef struct {
    i2c_master_bus_handle_t bus;
    i2c_master_dev_handle_t dev;
} lcd_i2c_t;

esp_err_t lcd_i2c_init(lcd_i2c_t *lcd, gpio_num_t sda, gpio_num_t scl, uint8_t addr);
void     lcd_i2c_clear(lcd_i2c_t *lcd);
void     lcd_i2c_set_cursor(lcd_i2c_t *lcd, int row, int col);
void     lcd_i2c_print(lcd_i2c_t *lcd, const char *str);
void     lcd_i2c_show(lcd_i2c_t *lcd, const char *line1, const char *line2);
```

The `.c` file then `#include`s its own header and implements these functions. Other files that need the LCD just `#include "lcd_i2c.h"`.

**Why this matters:** Header files are how C programmers define interfaces. Not having them is like writing a library without a README — nobody knows what functions to call or what parameters they take.

---

### 4. `sdkconfig` Duplicated 6 Times (~490 KB of Config Noise)

**Current approach:** Each project has its own `sdkconfig` file (~82 KB each), plus some have `sdkconfig.old` backups.

**Why this is a problem:**

- **Massive duplication** — These files are nearly identical. They all configure the same ESP32 chip with mostly default settings.
- **Maintenance burden** — If you need to change a setting (e.g., enable a specific compiler warning, change flash size), you must edit 6 files.
- **`sdkconfig.old` files** — These are auto-generated backups that ESP-IDF creates when you change settings. They should never be in version control.
- **Hard to see what's custom** — When every project has a full 82KB `sdkconfig`, it's impossible to tell at a glance what settings were actually changed from defaults vs. left at defaults.

**Better approach — use `sdkconfig.defaults`:**

Instead of committing the full generated `sdkconfig`, commit a small `sdkconfig.defaults` file that only contains the settings you explicitly changed:

```ini
# sdkconfig.defaults — only non-default settings
CONFIG_IDF_TARGET="esp32"
CONFIG_PARTITION_TABLE_CUSTOM=y
CONFIG_PARTITION_TABLE_CUSTOM_FILENAME="partitions.csv"
CONFIG_SPIRAM=y
```

ESP-IDF will read this file and generate the full `sdkconfig` automatically. Then add `sdkconfig` and `sdkconfig.old` to `.gitignore`.

**Why this matters:** In a real project with 10+ developers, having a minimal `sdkconfig.defaults` makes it obvious which settings are intentional vs. default. It also prevents merge conflicts on the massive auto-generated file.

---

### 5. No `.gitignore` — Build Artifacts in Version Control

**Current approach:** There is no `.gitignore` file. The `led_blink/main/build/` directory is committed to Git. All `sdkconfig` and `sdkconfig.old` files are tracked.

**Why this is a problem:**

- **Build artifacts don't belong in Git** — They're generated from source, they're platform-specific, and they bloat the repository. The `build/` directory can be hundreds of MB.
- **Merge conflicts** — If two people build at the same time, Git will see changes in `build/` and create conflicts.
- **Repository size** — Build artifacts make the repo grow permanently. Even if you delete them later, they stay in Git history.
- **`sdkconfig.old`** — These are stale backups that serve no purpose in version control.

**Suggested `.gitignore`:**

```gitignore
# ESP-IDF build output
build/
managed_components/
dependencies.lock

# SDK config backups
sdkconfig.old

# Compiled binaries
*.bin
*.elf
*.map
*.o

# IDE files
.vscode/
.idea/

# OS files
.DS_Store
Thumbs.db
```

**Why this matters:** A clean repository is a professional repository. Committing build artifacts is one of the most common mistakes for Git beginners, and it's the easiest to fix.

---

### 6. Flat Root Directory — No Logical Grouping by Phase

**Current approach:** All 6 project folders sit at the root level with no grouping.

```
ICHS-Internship/
├── led_blink/
├── UART-output/
├── Buttons-test/
├── LCD-test/
├── spiffs_test/
└── Combined_Hardware_Test/
```

**Why this is a problem:**

- **No narrative structure** — The projects follow a learning progression (Phase 1 → Phase 2 → Phase 3), but the folder structure doesn't reflect this. A newcomer can't tell which projects are prerequisites for others.
- **Root is cluttered** — 6 project folders + `Drives` + `README.md` + `code_observations.md` = 9 items at the root level with no organization.
- **No separation between "learning exercises" and "final product"** — The `Combined_Hardware_Test` is the actual deliverable; the others are stepping stones. They all look equally important.

**Better approach — group by phase:**

```
ICHS-Internship/
├── phase1_toolchain/
│   └── led_blink/
├── phase2_bringup/
│   ├── uart_output/
│   ├── buttons/
│   ├── lcd/
│   └── spiffs/
├── phase3_integration/
│   └── combined_hardware_test/
└── components/                    # Shared across all phases
    ├── lcd_i2c/
    ├── buttons/
    └── spiffs_log/
```

**Why this matters:** Folder structure tells a story. When a reviewer or another intern opens the repo, the folder names should immediately communicate the project's progression and priorities.

---

### 7. Inconsistent Folder Naming — `kebab-case` vs `snake_case`

**Current approach:**

| Folder | Naming Style |
|---|---|
| `led_blink` | `snake_case` |
| `UART-output` | `kebab-case` with uppercase |
| `Buttons-test` | `kebab-case` with uppercase |
| `LCD-test` | `kebab-case` with uppercase |
| `spiffs_test` | `snake_case` |
| `Combined_Hardware_Test` | `snake_case` with uppercase |

**Why this is a problem:**

- **Inconsistency** — There's no single convention. Some folders use underscores, some use hyphens, some use mixed case, some use lowercase.
- **Shell issues** — Hyphens in filenames can be misinterpreted as flags in some shell contexts (e.g., `cd UART-output` is fine, but `ls -l UART-output` could be ambiguous).
- **ESP-IDF convention** — ESP-IDF itself uses `snake_case` for project and component names (e.g., `blink`, `hello_world`, `i2c_simple`).

**Suggested fix — use `snake_case` consistently:**

| Current | Renamed |
|---|---|
| `led_blink` | `led_blink` (already correct) |
| `UART-output` | `uart_output` |
| `Buttons-test` | `button_test` |
| `LCD-test` | `lcd_test` |
| `spiffs_test` | `spiffs_test` (already correct) |
| `Combined_Hardware_Test` | `combined_hardware_test` |

**Why this matters:** Consistent naming is the simplest thing you can do to make a codebase look professional. It shows attention to detail and makes the project easier to navigate.

---

### 8. The `Drives` File Has No Extension and Unclear Purpose

**Current approach:** A file named `Drives` (no extension) sits at the root with 3 lines of Google Drive links.

**Why this is a problem:**

- **No file extension** — Without `.md` or `.txt`, editors don't know how to highlight it, and it's not obvious it's a text file.
- **Vague name** — "Drives" doesn't clearly communicate what's inside. Someone unfamiliar with the project wouldn't know this contains Google Drive links.
- **Links belong in README** — These are project resources that should be documented in the README, not in a separate mystery file.

**Suggested fix:** Move the content into the README (which was already done in the README update) and delete the `Drives` file. Or if you want to keep it separate, rename it to `RESOURCES.md` or `LINKS.md`.

---

## Variable Definition Patterns

### 9. Global Variables Instead of Context Structs

**Current approach:**

```c
// LCD-test.c
i2c_master_bus_handle_t bus;       // File-scope global
i2c_master_dev_handle_t lcd;       // File-scope global

// Combined_Hardware_Test.c
i2c_master_bus_handle_t bus;       // Same globals, copy-pasted
i2c_master_dev_handle_t lcd;
static int log_count = 0;          // Global state
```

**Why this is a problem:**

- **Hidden coupling** — Every function that calls `lcd_write()` silently depends on the global `lcd` handle. You can't tell from the function signature what state it needs.
- **Not reentrant** — If you ever needed two LCDs (e.g., one on I2C0 and one on I2C1), the globals would conflict. You'd have to rewrite every function.
- **Hard to test** — In unit tests, you can't create an isolated LCD context because the handle is a global.
- **Initialization order** — Globals are initialized at program start, but the I2C bus isn't actually configured until `app_main()` runs. The handles are valid but point to nothing until then — a subtle source of bugs.

**Better approach — pass a context struct:**

```c
// lcd_i2c.h
typedef struct {
    i2c_master_bus_handle_t bus;
    i2c_master_dev_handle_t dev;
    uint8_t addr;
} lcd_i2c_t;

// lcd_i2c.c
void lcd_write(lcd_i2c_t *lcd, uint8_t data)
{
    i2c_master_transmit(lcd->dev, &data, 1, 1000 / portTICK_PERIOD_MS);
}

void lcd_print(lcd_i2c_t *lcd, const char *str)
{
    while (*str)
        lcd_data(lcd, *str++);
}
```

```c
// main.c
lcd_i2c_t lcd;
lcd_i2c_init(&lcd, GPIO_NUM_21, GPIO_NUM_22, 0x27);
lcd_print(&lcd, "Hello");
```

**Why this matters:** Context structs are the standard pattern in ESP-IDF drivers. Every official ESP-IDF driver (I2C, SPI, UART, etc.) uses this pattern. Learning it now will make the intern's code consistent with the framework's conventions.

---

### 10. Pin Definitions Scattered Per-File Instead of Centralized

**Current approach:** Each `.c` file defines its own pin macros at the top:

```c
// LCD-test.c
#define SDA_PIN GPIO_NUM_21
#define SCL_PIN GPIO_NUM_22
#define LCD_ADDR 0x27

// Buttons-test.c
#define BTN1 GPIO_NUM_25
#define BTN2 GPIO_NUM_26
#define BTN3 GPIO_NUM_27

// Combined_Hardware_Test.c
#define SDA_PIN GPIO_NUM_21    // Duplicated from LCD-test.c
#define SCL_PIN GPIO_NUM_22    // Duplicated
#define LCD_ADDR 0x27          // Duplicated
#define BTN1 GPIO_NUM_32       // Different from Buttons-test.c!
#define BTN2 GPIO_NUM_33       // Different!
```

**Why this is a problem:**

- **Pin definitions are duplicated** — `SDA_PIN`, `SCL_PIN`, and `LCD_ADDR` are defined identically in two files.
- **Same macro name, different value** — `BTN1` is `GPIO_NUM_25` in `Buttons-test.c` but `GPIO_NUM_32` in `Combined_Hardware_Test.c`. If these were ever merged, there would be a macro collision.
- **No single source of truth** — To know what pins the hardware uses, you have to open every `.c` file and look for `#define` blocks.
- **No board header file** — ESP-IDF projects typically have a `board.h` or `pin_config.h` that centralizes all hardware pin assignments.

**Better approach — create a `board.h`:**

```c
// board.h — All hardware pin assignments in one place
#pragma once
#include "driver/gpio.h"

// I2C bus
#define I2C_SDA_PIN       GPIO_NUM_21
#define I2C_SCL_PIN       GPIO_NUM_22
#define LCD_I2C_ADDR      0x27

// Buttons (standalone test configuration)
#define BTN1_PIN          GPIO_NUM_25
#define BTN2_PIN          GPIO_NUM_26
#define BTN3_PIN          GPIO_NUM_27

// Buttons (combined test configuration)
#define COMBINED_BTN1_PIN GPIO_NUM_32
#define COMBINED_BTN2_PIN GPIO_NUM_33

// LED
#define LED_PIN           GPIO_NUM_33
```

Then every `.c` file includes `board.h` instead of defining its own pins.

**Why this matters:** When hardware changes (e.g., you move the LCD to different pins), you update one file instead of hunting through the codebase. This is especially important in a team where an electrical engineer might change the pinout and the software engineer needs to update the code.

---

### 11. No `typedef` or Struct Grouping for Related Configuration

**Current approach:** Related configuration values are defined as loose individual `#define` macros with no grouping.

```c
#define SDA_PIN GPIO_NUM_21
#define SCL_PIN GPIO_NUM_22
#define LCD_ADDR 0x27
```

**Why this is a problem:**

- **No semantic grouping** — A reader can *guess* that `SDA_PIN`, `SCL_PIN`, and `LCD_ADDR` are related to the LCD, but there's nothing in the code that enforces or expresses this relationship.
- **Easy to mix up** — Nothing prevents you from accidentally using `SDA_PIN` when you meant `SCL_PIN`. They're just two `int` values to the compiler.

**Better approach — group related config in a struct or at least a config block:**

```c
// Option 1: Config struct (runtime configuration)
typedef struct {
    gpio_num_t sda_pin;
    gpio_num_t scl_pin;
    uint8_t    i2c_addr;
    uint32_t   i2c_freq_hz;
} lcd_config_t;

// Usage:
lcd_config_t cfg = {
    .sda_pin = GPIO_NUM_21,
    .scl_pin = GPIO_NUM_22,
    .i2c_addr = 0x27,
    .i2c_freq_hz = 100000,
};
lcd_i2c_init(&lcd, &cfg);
```

```c
// Option 2: At minimum, group with a comment block and consistent prefix
// --- LCD I2C Configuration ---
#define LCD_I2C_SDA_PIN    GPIO_NUM_21
#define LCD_I2C_SCL_PIN    GPIO_NUM_22
#define LCD_I2C_ADDR       0x27
#define LCD_I2C_FREQ_HZ    100000
```

**Why this matters:** Grouping related values makes code self-documenting. When you see `lcd_config_t`, you immediately know what configuration the LCD needs. When you see `LCD_I2C_SDA_PIN`, you know it's the SDA pin *for the LCD's I2C bus*, not some other I2C device.

---

### 12. `static` Used Inconsistently

**Current approach:**

```c
// Combined_Hardware_Test.c
i2c_master_bus_handle_t bus;          // NOT static — external linkage
i2c_master_dev_handle_t lcd;          // NOT static
static int log_count = 0;             // static — internal linkage

// LCD-test.c
i2c_master_bus_handle_t bus;          // NOT static
i2c_master_dev_handle_t lcd;          // NOT static
```

**Why this is a problem:**

- **Inconsistent linkage** — `log_count` is `static` (only visible in this file), but `bus` and `lcd` are not (visible to any file that declares them `extern`). There's no clear reason for the difference.
- **Accidental external linkage** — If another file accidentally declares `extern i2c_master_bus_handle_t bus;`, it will link to this global. This can cause subtle bugs when combining files.
- **Rule of thumb** — Any file-scope variable that doesn't *need* to be visible outside the file should be `static`. This is called "internal linkage" and it's a C best practice.

**Suggested fix:**

```c
static i2c_master_bus_handle_t s_bus;   // static + s_ prefix convention
static i2c_master_dev_handle_t s_lcd;
static int s_log_count = 0;
```

Or better, eliminate globals entirely by using context structs (see finding #9).

**Why this matters:** `static` is C's primary tool for encapsulation at the file level. Using it consistently prevents accidental name collisions and makes the code's intended scope explicit. The `s_` prefix is a common embedded convention for "static" variables.

---

## File Splitting Recommendations

### 13. The Combined Hardware Test Should Be Split Into Multiple Files

**Current approach:** `Combined_Hardware_Test.c` is 238 lines containing:

- LCD I2C driver (lines 10-76) — 67 lines
- Pin definitions (lines 78-81) — 4 lines
- Global state (lines 83-85) — 3 lines
- SPIFFS functions (lines 87-112) — 26 lines
- Button functions (lines 114-137) — 24 lines
- LCD helper functions (lines 139-155) — 17 lines
- `app_main` (lines 159-237) — 79 lines

**Why this is a problem:**

- **Mixed concerns** — LCD driving, file I/O, button reading, and application logic are all in one file. There's no separation between "driver code" and "application code."
- **Hard to navigate** — 238 lines isn't huge, but it's enough that you need to scroll to find things. As the project grows, this will become 500+ lines quickly.
- **Can't test independently** — You can't test the LCD driver without also compiling the SPIFFS and button code.
- **Can't reuse** — The SPIFFS logging code can't be reused in another project without copy-pasting.

**Recommended split:**

```
Combined_Hardware_Test/
└── main/
    ├── CMakeLists.txt
    ├── main.c                    # app_main() only — orchestration
    ├── lcd_i2c.h                 # LCD driver API
    ├── lcd_i2c.c                 # LCD driver implementation
    ├── buttons.h                 # Button API
    ├── buttons.c                 # Button implementation
    ├── spiffs_log.h              # SPIFFS logging API
    └── spiffs_log.c              # SPIFFS logging implementation
```

Or better, put the drivers in `components/` so they're shared across all projects.

**Why this matters:** Separation of concerns is the most fundamental principle in software architecture. Each file should have one job. This makes code easier to read, test, debug, and reuse.

---

### 14. LCD Driver Should Be Its Own Component

**What to extract:** All LCD-related code from `LCD-test.c` and `Combined_Hardware_Test.c`.

**Component structure:**

```
components/lcd_i2c/
├── CMakeLists.txt
├── include/
│   └── lcd_i2c.h
└── lcd_i2c.c
```

**`lcd_i2c.h` (public API):**

```c
#pragma once
#include "esp_err.h"
#include "driver/gpio.h"

typedef struct lcd_i2c {
    i2c_master_bus_handle_t bus;
    i2c_master_dev_handle_t dev;
} lcd_i2c_t;

esp_err_t lcd_i2c_init(lcd_i2c_t *lcd, gpio_num_t sda, gpio_num_t scl,
                       uint8_t addr, uint32_t clk_hz);
void     lcd_i2c_clear(lcd_i2c_t *lcd);
void     lcd_i2c_set_cursor(lcd_i2c_t *lcd, int row, int col);
void     lcd_i2c_print(lcd_i2c_t *lcd, const char *str);
void     lcd_i2c_show(lcd_i2c_t *lcd, const char *line1, const char *line2);
```

**Why this matters:** The LCD driver is the most complex piece of code in this project. It deserves its own module with a clean API. Once extracted, both `LCD-test` and `Combined_Hardware_Test` can use it without duplication.

---

### 15. Button Handling Should Be Its Own Component

**What to extract:** Button init and reading logic from `Buttons-test.c` and `Combined_Hardware_Test.c`.

**Component structure:**

```
components/buttons/
├── CMakeLists.txt
├── include/
│   └── buttons.h
└── buttons.c
```

**`buttons.h` (public API):**

```c
#pragma once
#include "driver/gpio.h"
#include <stdbool.h>

typedef struct {
    gpio_num_t pin;
    bool       last_state;
    uint32_t   last_press_us;
} button_t;

void button_init(button_t *btn, gpio_num_t pin);
bool button_is_pressed(const button_t *btn);
bool button_pressed_event(button_t *btn);  // Returns true on falling edge (with debounce)
```

**Why this matters:** Button debouncing is a common need in almost every embedded project. Having a reusable, properly debounced button component saves time and prevents the debounce bugs seen in the current code.

---

### 16. SPIFFS Logging Should Be Its Own Component

**What to extract:** SPIFFS init and `write_log()` from `Combined_Hardware_Test.c`.

**Component structure:**

```
components/spiffs_log/
├── CMakeLists.txt
├── include/
│   └── spiffs_log.h
└── spiffs_log.c
```

**`spiffs_log.h` (public API):**

```c
#pragma once
#include "esp_err.h"

esp_err_t spiffs_log_init(const char *base_path, const char *partition_label);
esp_err_t spiffs_log_write(const char *message);
esp_err_t spiffs_log_read_all(void);
int      spiffs_log_count(void);
```

**Why this matters:** Logging to flash is a feature that will be needed in virtually every future ICHS firmware. Making it a reusable component means the intern can drop it into any new project with two lines of code.

---

## Recommended Project Structure

### Option A — Single Project with Build Targets (Recommended)

This is the cleanest approach for a project of this size. One ESP-IDF project, multiple `main` source files, and you select which one to build via CMake.

```
ICHS-Internship/
├── CMakeLists.txt                    # Top-level project
├── sdkconfig.defaults                # Minimal config (only non-defaults)
├── partitions.csv                    # Shared partition table
├── .gitignore
├── README.md
├── code_observations.md
├── structure_review.md
│
├── components/
│   ├── lcd_i2c/
│   │   ├── CMakeLists.txt
│   │   ├── include/
│   │   │   └── lcd_i2c.h
│   │   └── lcd_i2c.c
│   ├── buttons/
│   │   ├── CMakeLists.txt
│   │   ├── include/
│   │   │   └── buttons.h
│   │   └── buttons.c
│   └── spiffs_log/
│       ├── CMakeLists.txt
│       ├── include/
│       │   └── spiffs_log.h
│       └── spiffs_log.c
│
└── main/
    ├── CMakeLists.txt
    ├── board.h                       # All pin definitions
    ├── main.c                        # Combined hardware test (default)
    ├── test_led_blink.c              # Individual tests, selected via CMake
    ├── test_uart.c
    ├── test_buttons.c
    ├── test_lcd.c
    └── test_spiffs.c
```

**How to select which test to build:**

In `main/CMakeLists.txt`:
```cmake
# Default: build the combined test
set(MAIN_SRC "main.c")

# Override with: idf.py build -DMAIN_SRC=test_lcd.c
idf_component_register(
    SRCS "${MAIN_SRC}"
    INCLUDE_DIRS "."
    REQUIRES lcd_i2c buttons spiffs_log
)
```

Build a specific test:
```bash
idf.py build -DMAIN_SRC=test_lcd.c
idf.py build -DMAIN_SRC=test_buttons.c
idf.py build -DMAIN_SRC=main.c        # Combined test
```

**Why this is the best option:**
- One `sdkconfig` to maintain
- One build system to configure
- Shared components automatically available
- No code duplication
- Easy to switch between tests
- Clean root directory

---

### Option B — Multi-Project with Shared Components

If you prefer to keep each test as a separate project (e.g., for different partition tables or different ESP-IDF versions), use a shared `components/` directory at the root.

```
ICHS-Internship/
├── .gitignore
├── README.md
├── sdkconfig.defaults                # Shared defaults
│
├── components/                       # Shared across all projects
│   ├── lcd_i2c/
│   ├── buttons/
│   └── spiffs_log/
│
├── phase1_toolchain/
│   └── led_blink/
│       ├── CMakeLists.txt
│       └── main/
│           ├── CMakeLists.txt
│           └── led_blink.c
│
├── phase2_bringup/
│   ├── uart_output/
│   ├── button_test/
│   ├── lcd_test/
│   └── spiffs_test/
│
└── phase3_integration/
    └── combined_hardware_test/
        ├── CMakeLists.txt
        ├── partitions.csv
        └── main/
            ├── CMakeLists.txt
            └── main.c               # Uses components/ — no copy-paste
```

**How ESP-IDF finds shared components:**

In each project's top-level `CMakeLists.txt`:
```cmake
cmake_minimum_required(VERSION 3.22)

# Tell ESP-IDF to look for components in the parent's components/ dir
set(EXTRA_COMPONENT_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/../../components")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(combined_hardware_test)
```

**Why this is a good option:**
- Each test remains independently buildable
- Components are shared — no copy-paste
- Phase-based grouping tells the project's story
- Still has some `sdkconfig` duplication, but `sdkconfig.defaults` minimizes it

---

## Why This Matters for an Intern

You might be thinking: *"These are just small test programs, why does the structure matter?"*

Here's why:

1. **Habits form early** — The patterns you practice now are the patterns you'll use in your first job. If you practice copy-pasting code instead of creating reusable components, that habit will follow you.

2. **Code reviews in industry** — In a real job, your code will be reviewed by senior engineers. A 238-line file with mixed concerns, no headers, and copy-pasted drivers will get significant pushback in a PR review.

3. **ESP-IDF component system** — This is how *every* real ESP-IDF project is structured. Learning it now means you won't have to relearn it on the job. Browse any open-source ESP-IDF project on GitHub and you'll see a `components/` directory.

4. **Debugging** — When something breaks, a well-structured project lets you isolate the problem. "The LCD isn't displaying" → you look at `lcd_i2c.c`. "Buttons aren't registering" → you look at `buttons.c`. In the current structure, everything is in one file and you have to read all of it.

5. **Scalability** — The `Combined_Hardware_Test` is 238 lines today. When the project adds Wi-Fi, GPS, sensors, and a state machine, it will be 1000+ lines. If you don't split it now, refactoring later will be much harder.

6. **Professionalism** — A clean repository with a `.gitignore`, consistent naming, shared components, and a logical folder structure signals that you understand software engineering, not just coding.

---

## Migration Checklist

If you want to refactor the project, here's a step-by-step checklist:

- [ ] **1. Create a `.gitignore`** — Stop committing build artifacts and `sdkconfig.old`
- [ ] **2. Delete `sdkconfig.old` files** from the repo
- [ ] **3. Delete `led_blink/main/build/`** from the repo
- [ ] **4. Rename all folders** to consistent `snake_case`
- [ ] **5. Create a `board.h`** with all pin definitions
- [ ] **6. Extract the LCD driver** into `components/lcd_i2c/` with a header file
- [ ] **7. Extract button handling** into `components/buttons/` with a header file
- [ ] **8. Extract SPIFFS logging** into `components/spiffs_log/` with a header file
- [ ] **9. Split `Combined_Hardware_Test.c`** into `main.c` + component dependencies
- [ ] **10. Replace `sdkconfig` with `sdkconfig.defaults`** — only keep non-default settings
- [ ] **11. Move Google Drive links** into the README and delete the `Drives` file
- [ ] **12. Group folders by phase** (`phase1_toolchain/`, `phase2_bringup/`, `phase3_integration/`)
- [ ] **13. Test each project still builds** after refactoring
- [ ] **14. Update `README.md`** to reflect the new structure

> **Tip:** Do these one at a time, commit after each step, and verify the build still works before moving to the next. Don't try to do everything in one giant commit.
