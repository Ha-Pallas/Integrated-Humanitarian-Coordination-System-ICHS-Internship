# Report Terminal — Task 3B: State Machine Firmware

## What this is

This is the firmware for the "brain" of the field terminal — the part that walks a field worker through filling out a report, screen by screen, using 4 buttons and a small LCD.

The goal for this task wasn't to build the real thing yet. It was to build the *skeleton*: get every screen, every button press, and every transition working correctly, but leave "saving" and "syncing" as fake placeholders (just prints text like `SAVING REPORT...`) instead of actually writing files or connecting to Wi-Fi. That way the whole menu flow can be tested and confirmed to feel right before adding the complicated stuff on top.

## How it's organized

This project reuses the shared components already built earlier in the internship, instead of copy-pasting driver code again:
components/
├── include/       → board.h (all pin numbers in one place)
├── lcd_i2c/        → LCD driver
├── buttons/        → debounced button reading (now supports 4 buttons)
└── spiffs_log/      → flash storage (mounted, not used for real writes yet)
phase4_state_machine/report_terminal/
└── main/main.c      → everything for this task lives here
## The buttons

Two new buttons were added for this task (on top of the 2 that already existed), since a real menu needs more than just "press to confirm":

| Button | What it does |
|---|---|
| SELECT | confirm / move forward |
| UP | move up in a list, increase a value |
| DOWN | move down in a list, decrease a value |
| BACK | go back one step |

## The state machine

The whole firmware is basically one big loop that asks "what screen am I on, and what button got pressed?" and decides what to do next. This came straight from the UML diagram from Task 3A — every box in that diagram is one of these states:

**Idle → Category Selection → Detail Entry (severity, then people affected, then location) → Confirmation → Saving → Sync Attempt → Sync Success/Failed → back to Idle**

Every screen also supports BACK now (that wasn't in the original diagram — added it during this task), so pressing the wrong button doesn't mean starting over; you can back out one step at a time instead.

A `FieldReport` struct holds the data being filled in (category, severity, people affected, location) while going through the screens.

```c
typedef struct {
    uint32_t report_id;
    uint32_t timestamp;
    ReportCategory category;
    uint8_t severity;          // 1-5 scale
    uint16_t people_affected;
    char location[32];
    char device_id[16];
} FieldReport;
```

## What's real and what's fake right now

| Thing | Real or placeholder? |
|---|---|
| Moving between screens with buttons | ✅ real |
| Debounced button presses | ✅ real |
| LCD actually updating correctly | ✅ real |
| Severity / people affected / category values | ✅ real, stored in the struct |
| Location | ⚠️ pick-from-a-list (Camp A/B/C/Other) rather than free text, since typing isn't possible with only 4 buttons |
| Saving the report | ⚠️ placeholder — just prints `SAVING REPORT...`, doesn't actually write to flash yet |
| Syncing | ⚠️ placeholder — alternates between "success" and "failed" automatically, no real Wi-Fi/USB check |

## How to build and run it
cd phase4_state_machine/report_terminal
idf.py set-target esp32
idf.py -p COM3 flash monitor
(swap `COM3` for whatever port the board shows up as)

## Bugs run into while building this (worth knowing about)

**The LCD showed garbage text.** Not every time — only in this specific project, even though the exact same LCD code worked fine in an earlier test project. Took some process-of-elimination to figure out: commented out pieces one at a time (SPIFFS first, then buttons) until isolating the actual cause — `buttons_init()` was running *after* the LCD got set up, and configuring those 4 GPIO pins right at that moment was enough to disturb the LCD's internal state. Moving `buttons_init()` to run *before* the I2C/LCD setup fixed it completely. Good reminder that init order matters, even for things that seem electrically unrelated.

**BACK button used to skip too far.** Originally, pressing BACK while filling in any of the 3 detail fields (severity/people/location) would jump all the way back to Category Selection, no matter which field was active. Fixed it so BACK now steps back one field at a time instead, which feels a lot more natural.

## Sample run (real UART log)

Actual trace from testing on the hardware — cycling through categories, backing out once, picking "Shelter", setting severity to 3 and people affected to 2, leaving location on the default, confirming, and saving:
SYSTEM START
[STATE] Entering IDLE
[EVENT] button -> event 2 in state 0
[STATE] Entering CATEGORY_SELECTION (index 0)
[EVENT] button -> event 0 in state 1
[STATE] Entering CATEGORY_SELECTION (index 4)
[EVENT] button -> event 0 in state 1
[STATE] Entering CATEGORY_SELECTION (index 3)
[EVENT] button -> event 0 in state 1
[STATE] Entering CATEGORY_SELECTION (index 2)
[EVENT] button -> event 0 in state 1
[STATE] Entering CATEGORY_SELECTION (index 1)
[EVENT] button -> event 0 in state 1
[STATE] Entering CATEGORY_SELECTION (index 0)
[EVENT] button -> event 1 in state 1
[STATE] Entering CATEGORY_SELECTION (index 1)
[EVENT] button -> event 1 in state 1
[STATE] Entering CATEGORY_SELECTION (index 2)
[EVENT] button -> event 1 in state 1
[STATE] Entering CATEGORY_SELECTION (index 3)
[EVENT] button -> event 2 in state 1
[REPORT] category set to Shelter
[STATE] Entering DETAIL_ENTRY (field 0)
[EVENT] button -> event 3 in state 2
[STATE] Entering CATEGORY_SELECTION (index 3)
[EVENT] button -> event 2 in state 1
[REPORT] category set to Shelter
[STATE] Entering DETAIL_ENTRY (field 0)
[EVENT] button -> event 0 in state 2
[STATE] Entering DETAIL_ENTRY (field 0)
[EVENT] button -> event 0 in state 2
[STATE] Entering DETAIL_ENTRY (field 0)
[EVENT] button -> event 2 in state 2
[STATE] Entering DETAIL_ENTRY (field 1)
[EVENT] button -> event 0 in state 2
[STATE] Entering DETAIL_ENTRY (field 1)
[EVENT] button -> event 0 in state 2
[STATE] Entering DETAIL_ENTRY (field 1)
[EVENT] button -> event 2 in state 2
[STATE] Entering DETAIL_ENTRY (field 2)
[EVENT] button -> event 2 in state 2
[STATE] Entering CONFIRMATION — Shelter, sev=3, people=2, loc=Camp A
[EVENT] button -> event 2 in state 3
[STATE] Entering SAVING — placeholder: report would be written here
SAVING REPORT...
[STATE] Entering SYNC_ATTEMPT — placeholder: would check WiFi/USB here
SYNCING REPORTS...
[STATE] Entering SYNC_SUCCESS — placeholder: buffer would clear here
SYNC SUCCESS
[STATE] Entering IDLE
(event 0 = UP, 1 = DOWN, 2 = SELECT, 3 = BACK · state 0 = Idle, 1 = Category Selection, 2 = Detail Entry, 3 = Confirmation)

## Still to do

- Real free-text (or GPS-based?) location instead of a preset list
- Actually write reports to SPIFFS in the Saving state
- Actually check Wi-Fi/USB in the Sync Attempt state