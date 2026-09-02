# Platform Pocket Docs

This folder is the product and learning companion to `src/main.cpp`.

Platform Pocket is intentionally compact on-device, so the docs explain both **what the product does** and **how the firmware is built** without forcing someone to read the source first.

## Start here

If you want to understand Platform Pocket as a product, start with:

1. [Complete Feature Guide](FEATURES.md) — every menu item and major capability, what it does, why it exists, and whether it is available today
2. [SD Workspace and Persistence](STORAGE_WORKSPACE.md) — how the 64 GB-class microSD card becomes persistent notes, diagnostics, and the foundation for future incident/runbook features
3. [Field Tools](FIELD_TOOLS.md) — focused reference for the networking and diagnostic tools

If you want to understand or extend the codebase, continue with:

4. [Architecture](architecture.md) — how the whole application fits together
5. [UI and Input](ui-and-input.md) — how the Cardputer screen and keyboard are handled
6. [Terminal](terminal.md) — how the local command console works
7. [Adding a Tool](adding-a-tool.md) — the practical pattern for extending Platform Pocket
8. [Build and Debug](build-and-debug.md) — how the firmware becomes something running on the device

## What the Feature Guide answers

For every user-facing feature, `FEATURES.md` explains:

- **What it is** — the plain-language purpose of the feature.
- **What it does** — the actual behavior available on the device.
- **Why it matters** — the real troubleshooting, learning, or platform-engineering problem it helps solve.
- **Status** — whether the feature is available in the current firmware or still planned.

That distinction is important: the docs do not present planned placeholders as finished functionality.

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
| `src/main.cpp` | Current firmware implementation and UI routing |
| `src/storage.*` | Persistent Cardputer ADV microSD workspace and storage services |
| `docs/FEATURES.md` | Product-level feature catalog and rationale |
| `docs/STORAGE_WORKSPACE.md` | Persistent storage architecture, commands, directories, and rationale |
| `docs/` | Architecture, UI, terminal, extension, build, and learning material |
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
- microSD/SPI persistence
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

For persistence, trace a quick note:

```text
Terminal: note replaced cable
      -> runTerminalCommand()
      -> PocketStorage::appendQuickNote(...)
      -> /platform-pocket/notes/inbox.md
```

Then trace the Back key or filesystem path in reverse. This makes the state-machine and persistence architecture click very quickly.

## Design rule

When adding something new, try to answer four questions:

1. What state owns this screen?
2. What data does the feature need to remember?
3. What function performs the work?
4. What function draws the result?

Then add a fifth product question:

5. **Why does this feature deserve space on a pocket-sized device?**

If those answers are clear, the feature usually fits Platform Pocket cleanly.
