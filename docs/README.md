# Platform Pocket Docs

This folder is the learning companion to `src/main.cpp`.

The firmware is intentionally compact. These docs explain the ideas that were previously carried by large comment blocks in the source code, but in a form that is easier to read, update, and follow from beginning to end.

## Start here

If you are new to the project, read these in order:

1. [Architecture](architecture.md) — how the whole application fits together
2. [UI and Input](ui-and-input.md) — how the Cardputer screen and keyboard are handled
3. [Terminal](terminal.md) — how the local command console works
4. [Adding a Tool](adding-a-tool.md) — the practical pattern for extending Platform Pocket
5. [Build and Debug](build-and-debug.md) — how the firmware becomes something running on the device

## The 30-second mental model

Platform Pocket is an Arduino program running on the ESP32-S3 inside the Cardputer ADV.

```text
Physical keyboard
      |
      v
M5Cardputer.update()
      |
      v
ScreenState decides what owns input
      |
      +---- Main menu
      +---- Section menu
      +---- Wi-Fi scanner
      +---- Tool page
      +---- Terminal
      +---- Settings screens
      |
      v
Drawing functions update the 240x135 display
```

There is no desktop operating system underneath Platform Pocket. The firmware *is* the application. `setup()` runs once at startup, then `loop()` runs repeatedly for as long as the device is powered.

## What lives where?

| Area | Responsibility |
|---|---|
| `platformio.ini` | Build environment, board target, library dependency, upload/monitor settings |
| `src/main.cpp` | Current firmware implementation |
| `docs/` | Explanations, diagrams, extension guides, and learning material |
| `assets/` | README/project imagery |

## Core concepts worth learning

You do not need to master all of C++ before working on this project. The most important ideas are:

- variables and constants
- arrays
- `enum`
- functions
- `if` / `else`
- `switch`
- references
- Arduino `setup()` and `loop()`
- application state
- event-driven keyboard input
- drawing coordinates
- ESP32 Wi-Fi APIs
- separating UI code from tool logic

## A good way to learn this repo

Pick one feature and trace it from key press to pixels.

For example, Wi-Fi Scanner:

```text
Network selected
      -> Enter
SECTION_MENU
      -> Wi-Fi Scanner selected
executeSelectedTool()
      -> scanWifiNetworks()
      -> WiFi.scanNetworks(...)
      -> SCREEN_WIFI_SCAN
      -> drawWifiResults()
```

Then trace the Back key in reverse. This makes the state-machine architecture click very quickly.

## Design rule

When adding something new, try to answer four questions:

1. What state owns this screen?
2. What data does the feature need to remember?
3. What function performs the work?
4. What function draws the result?

If those four answers are clear, the feature usually fits the architecture cleanly.
