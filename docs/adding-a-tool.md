# Adding a Tool

This is the practical extension guide for Platform Pocket.

The goal is to add features without turning `loop()` into one giant function or duplicating UI code everywhere.

## The four-part pattern

Before writing code, answer these questions:

1. **Where does the feature appear?** Main menu, submenu, terminal, or settings?
2. **Does it need its own state?** Static information usually does not; interactive screens usually do.
3. **What data must it remember?** Selection, text input, samples, connection status, etc.
4. **What function does the actual work?** Keep work separate from drawing when possible.

---

## Example A: add a simple static tool

Suppose you want a "Memory Tips" page under Tools.

### Step 1 — Add the menu label

Extend the Tools menu:

```cpp
const char *toolsMenu[] = {
    "IP Tools",
    "Subnet Helper",
    "Base Converter",
    "Password Generator",
    "System Info",
    "Memory Tips"
};

const int toolsMenuCount = 6;
```

The count must match the number of entries.

### Step 2 — Add a function

```cpp
void showMemoryTips()
{
    String text = "";
    text += "Heap = dynamic RAM\n";
    text += "Flash = program storage\n";
    text += "Prefer bounded buffers.";

    openTool("MEMORY TIPS", text);
}
```

### Step 3 — Route the selected option

Inside the Tools branch of `executeSelectedTool()`:

```cpp
case 5:
    showMemoryTips();
    break;
```

That is enough because a static information page can reuse `SCREEN_TOOL`.

---

## Example B: add an interactive screen

Suppose you want a live RSSI monitor.

A graph changes over time and may react to keys, so it deserves its own screen state.

### Step 1 — Add a screen state

```cpp
enum ScreenState
{
    // existing states...
    SCREEN_SIGNAL_MONITOR
};
```

### Step 2 — Add feature state

```cpp
int signalSamples[30];
int signalSampleCount = 0;
unsigned long lastSignalSample = 0;
```

This is the information the feature needs to remember between calls to `loop()`.

### Step 3 — Add a drawing function

```cpp
void drawSignalMonitor()
{
    drawHeader("SIGNAL MONITOR", "RSSI");

    // Draw labels and samples here.

    drawFooter("ESC Back");
}
```

### Step 4 — Add an open function

```cpp
void openSignalMonitor()
{
    signalSampleCount = 0;
    currentScreen = SCREEN_SIGNAL_MONITOR;
    drawSignalMonitor();
}
```

Initialization belongs here instead of being scattered through input handling.

### Step 5 — Route the menu option

```cpp
case 2:
    openSignalMonitor();
    break;
```

### Step 6 — Give the state an input/update branch

Inside `loop()`:

```cpp
else if (currentScreen == SCREEN_SIGNAL_MONITOR)
{
    if (millis() - lastSignalSample >= 500)
    {
        lastSignalSample = millis();
        // collect sample
        drawSignalMonitor();
    }

    if (M5Cardputer.Keyboard.keysState().esc)
    {
        currentScreen = SCREEN_SECTION_MENU;
        drawSectionMenu();
    }
}
```

Now the signal monitor owns its own lifecycle.

---

## Example C: expose a feature in the terminal too

Suppose the signal monitor already has a helper:

```cpp
int getCurrentSignal()
{
    return WiFi.RSSI();
}
```

The graphical screen can use it:

```cpp
int rssi = getCurrentSignal();
```

The terminal can reuse it:

```cpp
else if (command == "rssi")
{
    terminalPrint(String(getCurrentSignal()) + " dBm");
}
```

This is a better architecture than writing separate Wi-Fi logic for the menu and terminal.

---

## When should code become a helper?

Create a helper when two or more places need the same behavior, or when a function is doing too many unrelated things.

Good helper candidates:

```cpp
String getWifiStatusText();
String getDeviceSummary();
int getCurrentSignal();
void performWifiScan();
```

Then multiple interfaces can reuse them.

```text
                  +--> Wi-Fi screen
shared helper ----+--> Security dashboard
                  +--> Terminal command
```

---

## Menu counts and indexes

Menu arrays and router indexes must stay aligned.

If the array is:

```text
0  IP Tools
1  Subnet Helper
2  Base Converter
3  Password Generator
4  System Info
```

then:

```cpp
case 4:
    showSystemInfo();
```

refers to the fifth item because C++ arrays start at zero.

An easy bug is adding a label without updating the menu count or router.

---

## Static page or dedicated state?

Use this quick decision table:

| Feature | Dedicated state? | Why |
|---|---:|---|
| About page | No | Displays fixed text |
| Docker cheatsheet | No | Displays fixed text |
| Generated password | Usually no | Generate once, then display |
| Wi-Fi scan list | Yes | Cursor and scrolling |
| Terminal | Yes | Text entry and command execution |
| Brightness | Yes | Live adjustment |
| Theme picker | Yes | Live visual selection |
| Signal graph | Yes | Continuous updates |
| Note editor | Yes | Text editing and persistence |

---

## Keep `loop()` boring

A healthy embedded `loop()` should mostly route events.

Bad direction:

```cpp
if (key) {
    // 200 lines of Wi-Fi work
    // 100 lines of UI
    // storage code
    // parsing
}
```

Better direction:

```cpp
if (currentScreen == SCREEN_TERMINAL)
{
    handleTerminalInput(status);
}
```

Then the details live in named functions.

Readable function names are a form of documentation.

---

## Suggested feature file split later

`main.cpp` is still manageable for learning, but as Platform Pocket grows, a future refactor could become:

```text
src/
  main.cpp
  ui.cpp
  terminal.cpp
  network.cpp
  tools.cpp

include/
  ui.h
  terminal.h
  network.h
  tools.h
```

Do not split files just to make the project look sophisticated. Split them when a subsystem has a clear responsibility and `main.cpp` becomes difficult to navigate.

---

## Extension checklist

Before committing a new feature:

- [ ] menu label is added in the right place
- [ ] menu count still matches
- [ ] router points to the correct option index
- [ ] interactive screens have a dedicated `ScreenState`
- [ ] Back/Escape returns somewhere sensible
- [ ] long text is truncated or wrapped safely
- [ ] drawing uses shared UI colors/functions
- [ ] Wi-Fi or hardware work does not get duplicated unnecessarily
- [ ] terminal help is updated if a command was added
- [ ] README/docs are updated if behavior changed

If those boxes are checked, the feature probably fits Platform Pocket cleanly.
