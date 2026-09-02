# Platform Pocket Docs

This folder is the product and learning companion to `src/main.cpp`.

Platform Pocket is intentionally compact on-device, so the docs explain both **what the product does** and **how the firmware is built** without forcing someone to read the source first.

## Start here

If you want to understand Platform Pocket as a product, start with:

1. [Platform Pocket v0.8 Feature Guide](V08_FEATURES.md) — the current workstation features, new Terminal commands, what each feature does, and why it matters
2. [Pocket Workstation](WORKSTATION.md) — File Manager, full-screen Markdown Editor, Incident Notebook, Snapshot Compare, offline Runbook Library, and Pocket Troubleshooter
3. [Complete Feature Guide](FEATURES.md) — the earlier networking, security, Docker, Terminal, system, and storage feature catalog
4. [SD Workspace and Persistence](STORAGE_WORKSPACE.md) — how the 64 GB-class microSD card provides persistent notes, diagnostics, incidents, runbooks, and future workspace data
5. [Field Tools](FIELD_TOOLS.md) — focused reference for networking and diagnostic tools

If you want to understand or extend the codebase, continue with:

6. [Architecture](architecture.md) — how the whole application fits together
7. [UI and Input](ui-and-input.md) — how the Cardputer screen and keyboard are handled
8. [Terminal](terminal.md) — how the local command console works
9. [Adding a Tool](adding-a-tool.md) — the practical pattern for extending Platform Pocket
10. [Build and Debug](build-and-debug.md) — how the firmware becomes something running on the device

## Current release

The current firmware documented here is **Platform Pocket v0.8**.

v0.8 keeps the Terminal as a first-class interface and adds an SD-backed workstation layer. The 64 GB microSD card is used for durable data, while ESP32-S3 RAM is kept for bounded working state rather than treating removable storage like extra RAM.

## What the feature docs answer

For user-facing features, the docs explain:

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
      +---- Markdown editor
      +---- Settings screens
      |
      v
Drawing functions update the 240x135 display

Persistent workstation services
      |
      +---- notes
      +---- incidents
      +---- diagnostics
      +---- runbooks
      +---- files
      |
      v
64 GB-class microSD workspace
```

There is no desktop operating system underneath Platform Pocket. The firmware *is* the application. `setup()` runs once at startup, then `loop()` runs repeatedly for as long as the device is powered.

## What lives where?

| Area | Responsibility |
|---|---|
| `platformio.ini` | Build environment, board target, library dependency, upload/monitor settings |
| `src/main.cpp` | Current firmware implementation, UI routing, Terminal, and editor state |
| `src/storage.*` | Persistent Cardputer ADV microSD workspace and storage services |
| `src/workstation.*` | File, incident, snapshot comparison, runbook, and offline troubleshooting services |
| `docs/V08_FEATURES.md` | Current v0.8 user-facing feature guide and rationale |
| `docs/WORKSTATION.md` | v0.8 pocket-workstation workflows and design |
| `docs/FEATURES.md` | Earlier feature catalog and rationale |
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
- bounded RAM buffers
- separating UI code from storage and workstation services

## A good way to learn this repo

Pick one feature and trace it from key press to pixels or disk.

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

For the v0.8 workstation, trace an incident observation:

```text
Terminal: incident add DNS failed after AP move
      -> runTerminalCommand()
      -> PocketWorkstation::appendIncident(...)
      -> /platform-pocket/incidents/<active-incident>.md
```

Or trace the editor:

```text
Terminal: edit
      -> openEditor()
      -> SCREEN_EDITOR
      -> bounded editorBuffer
      -> PocketWorkstation::saveEditorNote(...)
      -> /platform-pocket/notes/editor.md
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
