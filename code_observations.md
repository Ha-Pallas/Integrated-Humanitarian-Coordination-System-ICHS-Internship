# Code Observations & Review

> A code quality review of all firmware modules in the ICHS internship repository.  
> Findings are organized by severity: **Critical**, **High**, **Medium**, **Low**, and **Info / Best Practice**.  
> Each finding includes the file, line numbers, a description of the issue, and a suggested fix.

---

## Table of Contents

- [Critical Findings](#critical-findings)
  - [1. Missing NULL Check on `fopen` — Potential Crash (spiffs_test.c)](#1-missing-null-check-on-fopen--potential-crash-spiffs_testc)
  - [2. Uninitialized Buffer Printed After Failed `fgets` (spiffs_test.c)](#2-uninitialized-buffer-printed-after-failed-fgets-spiffs_testc)
- [High Findings](#high-findings)
  - [3. I2C Transmit Return Values Ignored (LCD-test.c, Combined_Hardware_Test.c)](#3-i2c-transmit-return-values-ignored-lcd-testc-combined_hardware_testc)
  - [4. GPIO Config Return Values Ignored (Buttons-test.c, Combined_Hardware_Test.c)](#4-gpio-config-return-values-ignored-buttons-testc-combined_hardware_testc)
  - [5. Blocking Debounce Halts All Button Monitoring (Buttons-test.c)](#5-blocking-debounce-halts-all-button-monitoring-buttons-testc)
  - [6. No Debounce in Combined Hardware Test (Combined_Hardware_Test.c)](#6-no-debounce-in-combined-hardware-test-combined_hardware_testc)
  - [7. `fprintf` to File Without Error Check (spiffs_test.c)](#7-fprintf-to-file-without-error-check-spiffs_testc)
- [Medium Findings](#medium-findings)
  - [8. Excessive Delay for LCD Enable Pulse — 1000x Too Long (LCD-test.c, Combined_Hardware_Test.c)](#8-excessive-delay-for-lcd-enable-pulse--1000x-too-long-lcd-testc-combined_hardware_testc)
  - [9. LCD Display Cleared Every Update — Causes Flicker (LCD-test.c, Combined_Hardware_Test.c)](#9-lcd-display-cleared-every-update--causes-flicker-lcd-testc-combined_hardware_testc)
  - [10. No Bounds Checking on `lcd_set_cursor` (LCD-test.c, Combined_Hardware_Test.c)](#10-no-bounds-checking-on-lcd_set_cursor-lcd-testc-combined_hardware_testc)
  - [11. No Bounds Checking on `lcd_print` — String Can Overflow Display (LCD-test.c, Combined_Hardware_Test.c)](#11-no-bounds-checking-on-lcd_print--string-can-overflow-display-lcd-testc-combined_hardware_testc)
  - [12. File-Scope Global Variables for I2C/LCD Handles (LCD-test.c, Combined_Hardware_Test.c)](#12-file-scope-global-variables-for-i2clcd-handles-lcd-testc-combined_hardware_testc)
  - [13. Massive Code Duplication — LCD Driver Copy-Pasted (Combined_Hardware_Test.c)](#13-massive-code-duplication--lcd-driver-copy-pasted-combined_hardware_testc)
  - [14. Copy-Pasted Button Handling Blocks (Buttons-test.c)](#14-copy-pasted-button-handling-blocks-buttons-testc)
  - [15. `spiffs_init` Uses `ESP_ERROR_CHECK` but `spiffs_test` Uses Manual Check — Inconsistent Error Strategy](#15-spiffs_init-uses-esp_error_check-but-spiffs_test-uses-manual-check--inconsistent-error-strategy)
  - [16. `partition_label` Mismatch Between Projects (spiffs_test.c vs Combined_Hardware_Test.c)](#16-partition_label-mismatch-between-projects-spiffs_testc-vs-combined_hardware_testc)
- [Low Findings](#low-findings)
  - [17. Magic Numbers Throughout LCD Driver (LCD-test.c, Combined_Hardware_Test.c)](#17-magic-numbers-throughout-lcd-driver-lcd-testc-combined_hardware_testc)
  - [18. `int` Used Instead of `bool` for Boolean Returns (Combined_Hardware_Test.c)](#18-int-used-instead-of-bool-for-boolean-returns-combined_hardware_testc)
  - [19. `int` Used for Counter Instead of `uint32_t` (UART-output.c, Combined_Hardware_Test.c)](#19-int-used-for-counter-instead-of-uint32_t-uart-outputc-combined_hardware_testc)
  - [20. Missing `#include <stdbool.h>` When Using Boolean Logic (All Files)](#20-missing-include-stdboolh-when-using-boolean-logic-all-files)
  - [21. `lcd_init()` Returns `void` — No Way to Detect Init Failure (LCD-test.c, Combined_Hardware_Test.c)](#21-lcd_init-returns-void--no-way-to-detect-init-failure-lcd-testc-combined_hardware_testc)
  - [22. `snprintf` Used as Overkill for Simple String Copy (Combined_Hardware_Test.c)](#22-snprintf-used-as-overkill-for-simple-string-copy-combined_hardware_testc)
  - [23. `gpio_set_direction` Used Instead of Full `gpio_config` (led_blink.c)](#23-gpio_set_direction-used-instead-of-full-gpio_config-led_blinkc)
  - [24. No `gpio_reset_pin` Before Reconfiguring Pins (All GPIO Projects)](#24-no-gpio_reset_pin-before-reconfiguring-pins-all-gpio-projects)
- [Info / Best Practice Recommendations](#info--best-practice-recommendations)
  - [25. No `.gitignore` — Build Artifacts Committed to Git](#25-no-gitignore--build-artifacts-committed-to-git)
  - [26. `sdkconfig.old` Files Committed — Stale Config in Repo](#26-sdkconfigold-files-committed--stale-config-in-repo)
  - [27. No Header Files — All Code in Single `.c` Files](#27-no-header-files--all-code-in-single-c-files)
  - [28. `printf` Used Instead of `ESP_LOG` Macros (All Files)](#28-printf-used-instead-of-esp_log-macros-all-files)
  - [29. No `esp_vfs_spiffs_unregister` Cleanup (spiffs_test.c, Combined_Hardware_Test.c)](#29-no-esp_vfs_spiffs_unregister-cleanup-spiffs_testc-combined_hardware_testc)
  - [30. Inconsistent Naming Conventions Across Projects](#30-inconsistent-naming-conventions-across-projects)
  - [31. No Comments Explaining "Why" — Only "What"](#31-no-comments-explaining-why--only-what)
- [Summary Table](#summary-table)
- [Recommended Reading](#recommended-reading)

---

## Critical Findings

### 1. Missing NULL Check on `fopen` — Potential Crash

**Files:** `spiffs_test/main/spiffs_test.c:22-24`, `spiffs_test/main/spiffs_test.c:31-33`, `spiffs_test/main/spiffs_test.c:38-40`, `spiffs_test/main/spiffs_test.c:47-53`

`fopen` can return `NULL` if the file cannot be opened (e.g., SPIFFS full, path too long, permission issue). The code immediately passes the result to `fprintf` or `fgets` without any NULL check, which would cause a **null pointer dereference crash**.

**Problematic code:**
```c
// spiffs_test.c:22-24
FILE *f = fopen("/spiffs/report.txt", "w");
fprintf(f, "report_001: TEST DATA\n");  // Crash if f == NULL
fclose(f);
```

```c
// spiffs_test.c:31-33
f = fopen("/spiffs/report.txt", "r");
fgets(buf, sizeof(buf), f);  // Crash if f == NULL
fclose(f);
```

**Suggested fix:**
```c
FILE *f = fopen("/spiffs/report.txt", "w");
if (f == NULL) {
    printf("Failed to open file for writing\n");
    return;
}
fprintf(f, "report_001: TEST DATA\n");
fclose(f);
```

> **Note:** The `Combined_Hardware_Test.c` `write_log()` function at line 104-108 *does* check for NULL — this is the correct pattern. Apply it everywhere.

---

### 2. Uninitialized Buffer Printed After Failed `fgets`

**File:** `spiffs_test/main/spiffs_test.c:29-35`

If `fgets` returns `NULL` (EOF or error), the buffer `buf` remains uninitialized. The very next line prints it with `printf`, reading garbage memory.

**Problematic code:**
```c
char buf[128];                        // uninitialized

f = fopen("/spiffs/report.txt", "r");
fgets(buf, sizeof(buf), f);           // may return NULL, buf stays uninitialized
fclose(f);

printf("READ: %s", buf);              // UB if fgets failed
```

**Suggested fix:**
```c
char buf[128] = {0};                  // zero-initialize

f = fopen("/spiffs/report.txt", "r");
if (f == NULL) {
    printf("Failed to open file for reading\n");
    return;
}
if (fgets(buf, sizeof(buf), f) != NULL) {
    printf("READ: %s", buf);
}
fclose(f);
```

---

## High Findings

### 3. I2C Transmit Return Values Ignored

**Files:** `LCD-test/main/LCD-test.c:16`, `Combined_Hardware_Test/main/Combined_Hardware_Test.c:23`

`i2c_master_transmit()` returns `esp_err_t` but the return value is never checked. If the LCD is disconnected, the I2C bus hangs, or the device NACKs, the error is silently swallowed and the code continues as if nothing happened.

**Problematic code:**
```c
void lcd_write(uint8_t data)
{
    i2c_master_transmit(lcd, &data, 1, 1000 / portTICK_PERIOD_MS);
    // return value ignored
}
```

**Suggested fix:**
```c
void lcd_write(uint8_t data)
{
    esp_err_t ret = i2c_master_transmit(lcd, &data, 1, 1000 / portTICK_PERIOD_MS);
    if (ret != ESP_OK) {
        ESP_LOGE("LCD", "I2C transmit failed: %s", esp_err_to_name(ret));
    }
}
```

Or, if you want to abort on failure:
```c
ESP_ERROR_CHECK(i2c_master_transmit(lcd, &data, 1, 1000 / portTICK_PERIOD_MS));
```

---

### 4. GPIO Config Return Values Ignored

**Files:** `Buttons-test/main/Buttons-test.c:20`, `Combined_Hardware_Test/main/Combined_Hardware_Test.c:126`

`gpio_config()` returns `esp_err_t` but the return value is never checked. If the GPIO configuration fails (e.g., invalid pin), the code proceeds to read from unconfigured pins.

**Problematic code:**
```c
// Buttons-test.c:20
gpio_config(&io);
// return value ignored
```

```c
// Combined_Hardware_Test.c:126
gpio_config(&io);
// return value ignored
```

**Suggested fix:**
```c
ESP_ERROR_CHECK(gpio_config(&io));
```

---

### 5. Blocking Debounce Halts All Button Monitoring

**File:** `Buttons-test/main/Buttons-test.c:39-58`

The debounce implementation uses `vTaskDelay(pdMS_TO_TICKS(DEBOUNCE_MS))` (200ms) *inside* the press detection block. This means:

1. **All button monitoring stops for 200ms** after any button press. If Button 1 is pressed, Buttons 2 and 3 cannot be read for 200ms.
2. **The button state is stale after the delay.** `last1 = b1` is set *after* the 200ms delay, but `b1` was read *before* the delay. The button may have been released during the delay, but `last1` is still set to 0, so the next press won't be detected until the button is physically released and pressed again.

**Problematic code:**
```c
if (b1 == 0 && last1 == 1) {
    printf("Button 1 pressed\n");
    vTaskDelay(pdMS_TO_TICKS(DEBOUNCE_MS));  // Blocks ALL buttons for 200ms
}
last1 = b1;  // b1 is stale — read before the delay
```

**Suggested fix — non-blocking debounce with timestamps:**
```c
#include "esp_timer.h"

uint32_t last_press_time[3] = {0, 0, 0};
#define DEBOUNCE_US 200000  // 200ms in microseconds

// In the loop:
int buttons[3] = {gpio_get_level(BTN1), gpio_get_level(BTN2), gpio_get_level(BTN3)};
uint32_t now = esp_timer_get_time();

for (int i = 0; i < 3; i++) {
    if (buttons[i] == 0 && last[i] == 1) {
        if (now - last_press_time[i] > DEBOUNCE_US) {
            printf("Button %d pressed\n", i + 1);
            last_press_time[i] = now;
        }
    }
    last[i] = buttons[i];
}
vTaskDelay(pdMS_TO_TICKS(20));
```

This approach never blocks and allows all buttons to be monitored continuously.

---

### 6. No Debounce in Combined Hardware Test

**File:** `Combined_Hardware_Test/main/Combined_Hardware_Test.c:203-236`

Unlike `Buttons-test.c` which at least *attempts* debounce (albeit with a flawed implementation), `Combined_Hardware_Test.c` has **zero debounce logic**. The 50ms polling loop with simple edge detection (`b1 && !last1`) will register multiple presses from a single physical button press due to mechanical contact bounce.

**Problematic code:**
```c
while (1)
{
    int b1 = btn1_pressed();
    int b2 = btn2_pressed();

    if (b1 && !last1) {   // No debounce — bounce will trigger this multiple times
        // ...
    }

    last1 = b1;
    last2 = b2;

    vTaskDelay(pdMS_TO_TICKS(50));  // 50ms poll — bounce can span multiple polls
}
```

**Suggested fix:** Implement the same non-blocking timestamp-based debounce described in finding #5.

---

### 7. `fprintf` to File Without Error Check

**File:** `spiffs_test/main/spiffs_test.c:23`, `spiffs_test/main/spiffs_test.c:39`

Even though `fopen` is the more likely failure point, `fprintf` can also fail (e.g., flash write error, disk full). The return value is never checked.

**Problematic code:**
```c
fprintf(f, "report_001: TEST DATA\n");  // return value ignored
```

**Suggested fix:**
```c
int written = fprintf(f, "report_001: TEST DATA\n");
if (written < 0) {
    printf("Failed to write to file\n");
}
```

---

## Medium Findings

### 8. Excessive Delay for LCD Enable Pulse — 1000x Too Long

**Files:** `LCD-test/main/LCD-test.c:23-25`, `Combined_Hardware_Test/main/Combined_Hardware_Test.c:29-31`

The HD44780 LCD controller requires an enable pulse of approximately **1 microsecond**. The code uses `vTaskDelay(pdMS_TO_TICKS(1))` which is a **1 millisecond** RTOS delay — 1000x longer than necessary. This makes every LCD operation unnecessarily slow.

**Problematic code:**
```c
void lcd_pulse(uint8_t data)
{
    lcd_write(data | 0x04);   // EN = 1
    vTaskDelay(pdMS_TO_TICKS(1));  // 1 ms — should be ~1 us
    lcd_write(data & ~0x04);  // EN = 0
    vTaskDelay(pdMS_TO_TICKS(1));  // 1 ms — should be ~1 us
}
```

Each `lcd_cmd` or `lcd_data` call sends 2 nibbles, each requiring 2 I2C writes + 2 delays = **4ms of delay per character**. Printing a 16-character line takes ~64ms just in delays.

**Suggested fix:** Use `esp_rom_delay_us()` for microsecond-level delays:
```c
#include "rom/ets_sys.h"

void lcd_pulse(uint8_t data)
{
    lcd_write(data | 0x04);
    ets_delay_us(1);    // 1 microsecond
    lcd_write(data & ~0x04);
    ets_delay_us(1);    // 1 microsecond
}
```

> **Note:** `ets_delay_us` is a busy-wait delay and does not yield to the RTOS scheduler. For 1µs this is perfectly fine. Do not use it for long delays.

---

### 9. LCD Display Cleared Every Update — Causes Flicker

**Files:** `LCD-test/main/LCD-test.c:109-110`, `Combined_Hardware_Test/main/Combined_Hardware_Test.c:145-146`

The code calls `lcd_cmd(0x01)` (clear display) on every update cycle. The HD44780 clear command takes ~1.5ms to execute and causes a visible flicker because the display is blank during that time.

**Problematic code:**
```c
while (1) {
    lcd_cmd(0x01); // clear — flicker!
    vTaskDelay(pdMS_TO_TICKS(5));

    lcd_set_cursor(0, 0);
    lcd_print("ICHS OK");
    // ...
}
```

**Suggested fix:** Only update the characters that changed. Instead of clearing, overwrite the existing content and pad with spaces:
```c
// Only clear once at init, then just overwrite
lcd_set_cursor(1, 0);
snprintf(buf, sizeof(buf), "Counter: %-6d", counter++);  // pad to overwrite old digits
lcd_print(buf);
```

---

### 10. No Bounds Checking on `lcd_set_cursor`

**Files:** `LCD-test/main/LCD-test.c:65-69`, `Combined_Hardware_Test/main/Combined_Hardware_Test.c:66-70`

If `row` is > 1 or `col` is > 15, the computed DDRAM address will be invalid, potentially corrupting the LCD's internal state or writing to unexpected memory locations.

**Problematic code:**
```c
void lcd_set_cursor(int row, int col)
{
    int addr = (row == 0) ? 0x80 + col : 0xC0 + col;
    lcd_cmd(addr);
}
```

**Suggested fix:**
```c
void lcd_set_cursor(int row, int col)
{
    if (row < 0 || row > 1 || col < 0 || col > 15) {
        return;  // or log an error
    }
    int addr = (row == 0) ? 0x80 + col : 0xC0 + col;
    lcd_cmd(addr);
}
```

---

### 11. No Bounds Checking on `lcd_print` — String Can Overflow Display

**Files:** `LCD-test/main/LCD-test.c:72-76`, `Combined_Hardware_Test/main/Combined_Hardware_Test.c:72-76`

`lcd_print` sends characters to the LCD without limiting to the 16-column display width. If a string longer than 16 characters is passed, the extra characters will wrap to the next line or write to invisible DDRAM addresses, causing garbled display behavior.

**Problematic code:**
```c
void lcd_print(const char *str)
{
    while (*str)
        lcd_data(*str++);  // no limit — will overflow past 16 chars
}
```

**Suggested fix:**
```c
void lcd_print(const char *str)
{
    int i = 0;
    while (*str && i < 16) {
        lcd_data(*str++);
        i++;
    }
}
```

---

### 12. File-Scope Global Variables for I2C/LCD Handles

**Files:** `LCD-test/main/LCD-test.c:10-11`, `Combined_Hardware_Test/main/Combined_Hardware_Test.c:18-19`

The I2C bus handle and LCD device handle are declared as file-scope globals. This makes them accessible from anywhere in the file and creates hidden coupling. If the code is ever split into multiple files or if multiple LCDs are needed, this pattern breaks down.

**Problematic code:**
```c
i2c_master_bus_handle_t bus;
i2c_master_dev_handle_t lcd;
```

**Suggested fix:** Encapsulate in a struct and pass as a parameter:
```c
typedef struct {
    i2c_master_bus_handle_t bus;
    i2c_master_dev_handle_t lcd;
} lcd_ctx_t;

void lcd_write(lcd_ctx_t *ctx, uint8_t data) {
    i2c_master_transmit(ctx->lcd, &data, 1, 1000 / portTICK_PERIOD_MS);
}
// ... etc
```

---

### 13. Massive Code Duplication — LCD Driver Copy-Pasted

**File:** `Combined_Hardware_Test/main/Combined_Hardware_Test.c:10-76`

The entire LCD driver (~67 lines) is copy-pasted verbatim from `LCD-test.c` into `Combined_Hardware_Test.c`. If a bug is found in the LCD driver, it must be fixed in two places. If a third project needs the LCD, it would be copied again.

**Suggested fix:** Create a reusable ESP-IDF component:
```
components/
└── lcd_i2c/
    ├── CMakeLists.txt
    ├── lcd_i2c.h
    └── lcd_i2c.c
```

Then include it in each project's `CMakeLists.txt`:
```cmake
idf_component_register(SRCS "Combined_Hardware_Test.c"
                    INCLUDE_DIRS "."
                    REQUIRES lcd_i2c)
```

---

### 14. Copy-Pasted Button Handling Blocks

**File:** `Buttons-test/main/Buttons-test.c:39-58`

The three button-handling blocks are identical copy-paste code, differing only in the variable names (`b1`/`last1`, `b2`/`last2`, `b3`/`last3`) and the button number in the printf string.

**Problematic code:**
```c
// BUTTON 1
if (b1 == 0 && last1 == 1) {
    printf("Button 1 pressed\n");
    vTaskDelay(pdMS_TO_TICKS(DEBOUNCE_MS));
}
last1 = b1;

// BUTTON 2
if (b2 == 0 && last2 == 1) {
    printf("Button 2 pressed\n");
    vTaskDelay(pdMS_TO_TICKS(DEBOUNCE_MS));
}
last2 = b2;

// BUTTON 3
if (b3 == 0 && last3 == 1) {
    printf("Button 3 pressed\n");
    vTaskDelay(pdMS_TO_TICKS(DEBOUNCE_MS));
}
last3 = b3;
```

**Suggested fix — use arrays and a loop:**
```c
gpio_num_t btn_pins[] = {BTN1, BTN2, BTN3};
int last[] = {1, 1, 1};

while (1) {
    for (int i = 0; i < 3; i++) {
        int val = gpio_get_level(btn_pins[i]);
        if (val == 0 && last[i] == 1) {
            printf("Button %d pressed\n", i + 1);
            vTaskDelay(pdMS_TO_TICKS(DEBOUNCE_MS));
        }
        last[i] = val;
    }
    vTaskDelay(pdMS_TO_TICKS(20));
}
```

---

### 15. `spiffs_init` Uses `ESP_ERROR_CHECK` but `spiffs_test` Uses Manual Check — Inconsistent Error Strategy

**Files:** `spiffs_test/main/spiffs_test.c:14-17` vs `Combined_Hardware_Test/main/Combined_Hardware_Test.c:98`

The two SPIFFS initialization approaches are inconsistent:

```c
// spiffs_test.c — manual check, graceful return
if (esp_vfs_spiffs_register(&conf) != ESP_OK) {
    printf("SPIFFS mount FAILED\n");
    return;
}
```

```c
// Combined_Hardware_Test.c — abort on failure
ESP_ERROR_CHECK(esp_vfs_spiffs_register(&conf));
```

`ESP_ERROR_CHECK` will trigger an abort and reboot on failure. The manual check allows for graceful degradation. Pick one strategy and use it consistently. For a field-deployed humanitarian device, graceful error handling is usually preferable over a reboot loop.

---

### 16. `partition_label` Mismatch Between Projects

**Files:** `spiffs_test/main/spiffs_test.c:9` vs `Combined_Hardware_Test/main/Combined_Hardware_Test.c:93`

The SPIFFS partition label is set differently in the two projects:

```c
// spiffs_test.c — explicitly references "storage" label
.partition_label = "storage",
```

```c
// Combined_Hardware_Test.c — uses NULL (default label)
.partition_label = NULL,
```

Both projects use the same `partitions.csv` with a partition named `storage`. Using `NULL` will fall back to the default label, which may or may not match `"storage"` depending on the ESP-IDF version. This should be consistent.

**Suggested fix:** Use `"storage"` explicitly in both projects.

---

## Low Findings

### 17. Magic Numbers Throughout LCD Driver

**Files:** `LCD-test/main/LCD-test.c:22-25,31,54-61`, `Combined_Hardware_Test/main/Combined_Hardware_Test.c:28-31,36,56-61`

The LCD driver is full of magic numbers with no named constants:

| Magic Value | Meaning |
|---|---|
| `0x04` | Enable (EN) bit on PCF8574 |
| `0x08` | Backlight bit on PCF8574 |
| `0x01` | Register Select (RS) bit on PCF8574 |
| `0x33`, `0x32` | 4-bit initialization sequence |
| `0x28` | 2-line, 5×8 font mode |
| `0x0C` | Display on, cursor off |
| `0x06` | Entry mode: increment, no shift |
| `0x01` | Clear display command |
| `0x80`, `0xC0` | DDRAM address offsets for row 0 / row 1 |

**Suggested fix — define named constants:**
```c
#define LCD_EN_BIT    0x04
#define LCD_RW_BIT    0x02
#define LCD_RS_BIT    0x01
#define LCD_BL_BIT    0x08

#define LCD_CMD_CLEAR      0x01
#define LCD_CMD_HOME       0x02
#define LCD_CMD_ENTRYMODE  0x06
#define LCD_CMD_DISPLAY    0x0C
#define LCD_CMD_FUNCTION   0x28
#define LCD_CMD_4BIT_INIT1 0x33
#define LCD_CMD_4BIT_INIT2 0x32

#define LCD_ROW0_ADDR  0x80
#define LCD_ROW1_ADDR  0xC0
#define LCD_COLS       16
#define LCD_ROWS       2
```

---

### 18. `int` Used Instead of `bool` for Boolean Returns

**File:** `Combined_Hardware_Test/main/Combined_Hardware_Test.c:129-137`

`btn1_pressed()` and `btn2_pressed()` return `int` (0 or 1) instead of `bool`.

**Problematic code:**
```c
int btn1_pressed()
{
    return gpio_get_level(BTN1) == 0;
}
```

**Suggested fix:**
```c
#include <stdbool.h>

bool btn1_pressed(void)
{
    return gpio_get_level(BTN1) == 0;
}
```

---

### 19. `int` Used for Counter Instead of `uint32_t`

**Files:** `UART-output/main/UART-output.c:7`, `Combined_Hardware_Test/main/Combined_Hardware_Test.c:85`

Counters that are never negative should use unsigned types. While `int` overflow at 1 increment/second takes ~68 years, using `uint32_t` is the correct semantic choice and makes the intent clear.

**Suggested fix:**
```c
uint32_t counter = 0;    // instead of int counter = 0;
static uint32_t log_count = 0;  // instead of static int log_count = 0;
```

---

### 20. Missing `#include <stdbool.h>` When Using Boolean Logic

**Files:** All projects that use `int` as boolean

Several files use `int` for boolean logic (e.g., `last1 == 1`, `b1 && !last1`) without including `<stdbool.h>` and using the `bool`/`true`/`false` types. This is a C best practice for code clarity.

---

### 21. `lcd_init()` Returns `void` — No Way to Detect Init Failure

**Files:** `LCD-test/main/LCD-test.c:50`, `Combined_Hardware_Test/main/Combined_Hardware_Test.c:52`

If LCD initialization fails (e.g., I2C device not responding), there is no way for the caller to know. The function should return `esp_err_t`.

**Suggested fix:**
```c
esp_err_t lcd_init(void)
{
    // ... initialization commands ...
    // Check I2C transmit results
    return ESP_OK;
}
```

---

### 22. `snprintf` Used as Overkill for Simple String Copy

**File:** `Combined_Hardware_Test/main/Combined_Hardware_Test.c:153`

In `lcd_show()`, `snprintf` is used to copy a string into a buffer, which is then passed to `lcd_print`. This is unnecessarily heavy — `lcd_print` already takes a `const char *` and the intermediate buffer adds nothing.

**Problematic code:**
```c
void lcd_show(const char *line1, const char *line2)
{
    char buf[32];
    // ...
    snprintf(buf, sizeof(buf), "%s", line2);
    lcd_print(buf);
}
```

**Suggested fix — pass directly:**
```c
void lcd_show(const char *line1, const char *line2)
{
    // ...
    lcd_set_cursor(1, 0);
    lcd_print(line2);
}
```

The intermediate buffer was likely added to truncate long strings, but since `lcd_print` has no length limit anyway (see finding #11), the buffer doesn't actually protect against anything.

---

### 23. `gpio_set_direction` Used Instead of Full `gpio_config`

**File:** `led_blink/main/led_blink.c:9`

`gpio_set_direction()` is a simplified API that doesn't configure pull-up/pull-down resistors or interrupt type. While this is fine for a simple LED output, using `gpio_config()` with a full `gpio_config_t` struct is the more robust and explicit approach, and is consistent with what's done in `Buttons-test.c` and `Combined_Hardware_Test.c`.

---

### 24. No `gpio_reset_pin` Before Reconfiguring Pins

**Files:** All projects that configure GPIO pins

ESP-IDF recommends calling `gpio_reset_pin()` before configuring a GPIO pin, especially if the pin may have been configured by a previous boot or by the bootloader (e.g., strapping pins, flash pins). This ensures the pin starts from a clean state.

**Suggested fix:**
```c
gpio_reset_pin(LED_PIN);
gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);
```

---

## Info / Best Practice Recommendations

### 25. No `.gitignore` — Build Artifacts Committed to Git

**Repository root**

There is no `.gitignore` file in the repository. The `led_blink/main/build/` directory is present, which means compiled build artifacts are being tracked in version control. The `sdkconfig` files (~82KB each) and their `.old` backups are also committed.

**Suggested fix — create a `.gitignore` file:**
```gitignore
build/
sdkconfig.old
*.o
*.bin
*.elf
*.map
dependencies.lock
```

> **Note:** Some teams choose to commit `sdkconfig` for reproducible builds. This is a valid choice, but `sdkconfig.old` and `build/` should never be committed.

---

### 26. `sdkconfig.old` Files Committed — Stale Config in Repo

**Files:** `UART-output/sdkconfig.old`, `led_blink/sdkconfig.old`, `spiffs_test/sdkconfig.old`, `Combined_Hardware_Test/sdkconfig.old`

These are backup files generated by ESP-IDF when `sdkconfig` changes. They serve no purpose in version control and add ~80KB of noise each.

**Suggested fix:** Delete them from the repo and add `sdkconfig.old` to `.gitignore`.

---

### 27. No Header Files — All Code in Single `.c` Files

**All projects**

Every project is a single `.c` file with no corresponding `.h` header. While this works for small test programs, it prevents code reuse and makes the API unclear. The LCD driver in particular should be a separate component with a header file.

**Suggested structure for the LCD driver:**
```c
// lcd_i2c.h
#pragma once
#include "esp_err.h"
#include "driver/i2c_master.h"

typedef struct {
    i2c_master_bus_handle_t bus;
    i2c_master_dev_handle_t dev;
} lcd_i2c_handle_t;

esp_err_t lcd_i2c_init(lcd_i2c_handle_t *handle, gpio_num_t sda, gpio_num_t scl, uint8_t addr);
void lcd_i2c_clear(lcd_i2c_handle_t *handle);
void lcd_i2c_set_cursor(lcd_i2c_handle_t *handle, int row, int col);
void lcd_i2c_print(lcd_i2c_handle_t *handle, const char *str);
```

---

### 28. `printf` Used Instead of `ESP_LOG` Macros

**Files:** All projects

All projects use `printf()` for debug output. ESP-IDF provides a logging framework (`ESP_LOGI`, `ESP_LOGW`, `ESP_LOGE`, `ESP_LOGD`, `ESP_LOGV`) with several advantages:

- **Log levels** can be controlled at runtime (e.g., silence INFO logs in production)
- **Tag-based filtering** — you can enable/disable logs per module
- **Color-coded output** in the serial monitor
- **Timestamps** automatically included
- **Thread-safe** — uses a mutex internally

**Suggested fix:**
```c
#include "esp_log.h"

static const char *TAG = "BUTTONS";

// Instead of:
printf("Button 1 pressed\n");

// Use:
ESP_LOGI(TAG, "Button 1 pressed");

// Instead of:
printf("SPIFFS mount FAILED\n");

// Use:
ESP_LOGE(TAG, "SPIFFS mount FAILED");
```

---

### 29. No `esp_vfs_spiffs_unregister` Cleanup

**Files:** `spiffs_test/main/spiffs_test.c`, `Combined_Hardware_Test/main/Combined_Hardware_Test.c`

The SPIFFS filesystem is registered with `esp_vfs_spiffs_register()` but never unregistered with `esp_vfs_spiffs_unregister()`. For a program that runs forever, this is fine. But for a test program like `spiffs_test.c` where `app_main` returns, it's good practice to clean up resources.

---

### 30. Inconsistent Naming Conventions Across Projects

**All projects**

The codebase mixes several naming conventions:

| Style | Example | Location |
|---|---|---|
| `snake_case` | `led_blink.c`, `spiffs_test.c` | Filenames |
| `kebab-case` | `Buttons-test.c`, `LCD-test.c`, `UART-output.c` | Filenames |
| `snake_case` | `init_button()`, `write_log()` | Functions |
| `UPPER_SNAKE` | `BTN1`, `LED_PIN`, `LCD_ADDR` | Macros |
| `camelCase` | (not used) | — |

Filenames should be consistent — pick either `snake_case` or `kebab-case` and stick with it. ESP-IDF convention is `snake_case`.

---

### 31. No Comments Explaining "Why" — Only "What"

**All projects**

The code has some comments, but they describe *what* the code does (e.g., `// EN = 1`, `// clear`) rather than *why* it does it. In embedded systems, the "why" is critical because hardware timing requirements, datasheet references, and design decisions are not obvious from the code alone.

**Example of unhelpful comment:**
```c
lcd_write(data | 0x04);   // EN = 1
```

**Better comment:**
```c
lcd_write(data | LCD_EN_BIT);   // Pulse EN high to latch data on falling edge (HD44780 datasheet p.58)
```

**Good example of what to document:**
```c
// 4-bit init sequence: send 0x33 then 0x32 to force the HD44780 into
// 4-bit mode from an unknown power-on state (datasheet section 4.3)
lcd_cmd(0x33);
lcd_cmd(0x32);
```

---

## Summary Table

| # | Severity | File(s) | Issue |
|---|---|---|---|
| 1 | **Critical** | `spiffs_test.c` | Missing NULL check on `fopen` — potential crash |
| 2 | **Critical** | `spiffs_test.c` | Uninitialized buffer printed after failed `fgets` |
| 3 | **High** | `LCD-test.c`, `Combined_Hardware_Test.c` | I2C transmit return values ignored |
| 4 | **High** | `Buttons-test.c`, `Combined_Hardware_Test.c` | GPIO config return values ignored |
| 5 | **High** | `Buttons-test.c` | Blocking debounce halts all button monitoring |
| 6 | **High** | `Combined_Hardware_Test.c` | No debounce at all in combined test |
| 7 | **High** | `spiffs_test.c` | `fprintf` return value not checked |
| 8 | **Medium** | `LCD-test.c`, `Combined_Hardware_Test.c` | LCD enable pulse delay 1000x too long (1ms vs 1µs) |
| 9 | **Medium** | `LCD-test.c`, `Combined_Hardware_Test.c` | LCD cleared every update — causes flicker |
| 10 | **Medium** | `LCD-test.c`, `Combined_Hardware_Test.c` | No bounds checking on `lcd_set_cursor` |
| 11 | **Medium** | `LCD-test.c`, `Combined_Hardware_Test.c` | No bounds checking on `lcd_print` |
| 12 | **Medium** | `LCD-test.c`, `Combined_Hardware_Test.c` | File-scope global variables for I2C/LCD handles |
| 13 | **Medium** | `Combined_Hardware_Test.c` | Entire LCD driver copy-pasted — should be a component |
| 14 | **Medium** | `Buttons-test.c` | Three identical button-handling blocks — should use a loop |
| 15 | **Medium** | `spiffs_test.c`, `Combined_Hardware_Test.c` | Inconsistent error handling strategy |
| 16 | **Medium** | `spiffs_test.c`, `Combined_Hardware_Test.c` | `partition_label` mismatch (`"storage"` vs `NULL`) |
| 17 | **Low** | `LCD-test.c`, `Combined_Hardware_Test.c` | Magic numbers throughout LCD driver |
| 18 | **Low** | `Combined_Hardware_Test.c` | `int` instead of `bool` for boolean returns |
| 19 | **Low** | `UART-output.c`, `Combined_Hardware_Test.c` | `int` instead of `uint32_t` for counters |
| 20 | **Low** | All files | Missing `<stdbool.h>` when using boolean logic |
| 21 | **Low** | `LCD-test.c`, `Combined_Hardware_Test.c` | `lcd_init()` returns `void` — no failure detection |
| 22 | **Low** | `Combined_Hardware_Test.c` | `snprintf` used as overkill for simple string copy |
| 23 | **Low** | `led_blink.c` | `gpio_set_direction` instead of full `gpio_config` |
| 24 | **Low** | All GPIO projects | No `gpio_reset_pin` before reconfiguring pins |
| 25 | **Info** | Repository root | No `.gitignore` — build artifacts committed |
| 26 | **Info** | Multiple | `sdkconfig.old` files committed — stale config |
| 27 | **Info** | All projects | No header files — all code in single `.c` files |
| 28 | **Info** | All projects | `printf` instead of `ESP_LOG*` macros |
| 29 | **Info** | `spiffs_test.c`, `Combined_Hardware_Test.c` | No SPIFFS unregister cleanup |
| 30 | **Info** | All projects | Inconsistent naming conventions (snake_case vs kebab-case) |
| 31 | **Info** | All projects | Comments describe "what" not "why" |

---

## Recommended Reading

- [ESP-IDF API Reference — Error Handling](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/error-handling.html)
- [ESP-IDF API Reference — Logging](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/log.html)
- [ESP-IDF API Reference — GPIO](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/gpio.html)
- [ESP-IDF API Reference — I2C](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/i2c.html)
- [HD44780 LCD Controller Datasheet](https://www.sparkfun.com/datasheets/LCD/HD44780.pdf)
- [ESP-IDF Build System — Components](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/build-system.html#components)
- [FreeRTOS — Debouncing Buttons](https://www.freertos.org/index.html)
