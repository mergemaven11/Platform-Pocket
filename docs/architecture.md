# Architecture

This document explains how Platform Pocket fits together from power-on to pixels on the screen.

## 1. The runtime model

Platform Pocket uses the Arduino programming model:

```cpp
void setup()
{
    // Runs once.
}

void loop()
{
    // Runs forever.
}
```

On startup, `setup()` initializes the Cardputer, display orientation, brightness, theme, and first screen.

After that, `loop()` repeatedly:

1. updates the Cardputer hardware state
2. checks whether the keyboard changed
3. checks `currentScreen`
4. sends the input to the code responsible for that screen
5. redraws only when something meaningful happens

That last point matters on a small embedded device. Constantly repainting the whole display wastes work and can create flicker.

---

## 2. The state machine

The central architectural idea is `ScreenState`.

```cpp
enum ScreenState
{
    SCREEN_MAIN,
    SCREEN_SECTION_MENU,
    SCREEN_TOOL,
    SCREEN_WIFI_SCAN,
    SCREEN_WIFI_DETAILS,
    SCREEN_TERMINAL,
    SCREEN_BRIGHTNESS,
    SCREEN_THEME
};
```

`currentScreen` stores which screen currently owns the device.

Think of this as a traffic controller:

```text
                  +-------------------+
                  |    SCREEN_MAIN    |
                  +---------+---------+
                            |
                         Enter
                            v
                  +-------------------+
                  | SCREEN_SECTION... |
                  +--+------+-------+-+
                     |      |       |
               scanner   tool   terminal
                     |      |       |
                     v      v       v
                  Wi-Fi   Tool    Terminal
                  states   page     state
```

The state machine prevents every key from affecting every feature.

For example, `;` means "move up" on a menu, but inside the brightness screen the same physical key can mean "decrease brightness". The current state provides the context.

---

## 3. Main menu identifiers

`MainMenuItem` gives readable names to the top-level sections:

```cpp
enum MainMenuItem
{
    MAIN_NETWORK,
    MAIN_SECURITY,
    MAIN_DOCKER,
    MAIN_TERMINAL,
    MAIN_NOTES,
    MAIN_TOOLS,
    MAIN_SETTINGS,
    MAIN_COUNT
};
```

Enums are integers underneath, but names make the code easier to understand.

This:

```cpp
if (selectedMainItem == MAIN_DOCKER)
```

is much clearer than:

```cpp
if (selectedMainItem == 2)
```

`MAIN_COUNT` is especially useful because it automatically represents the number of top-level items.

---

## 4. Data versus state

These sound similar but serve different jobs.

### Data

Data describes what the application contains.

```cpp
const char *mainMenuItems[] = {
    "Network",
    "Security",
    "Docker",
    "Terminal",
    "Notes",
    "Tools",
    "Settings"
};
```

### State

State describes what the application is doing *right now*.

```cpp
ScreenState currentScreen = SCREEN_MAIN;
int selectedMainItem = 0;
int selectedSubItem[MAIN_COUNT] = {0, 0, 0, 0, 0, 0, 0};
```

A useful mental rule:

> Data says what exists. State says where the user currently is.

---

## 5. Why each section remembers its selection

`selectedSubItem` is an array indexed by the main-menu enum.

```cpp
selectedSubItem[MAIN_NETWORK]
selectedSubItem[MAIN_DOCKER]
selectedSubItem[MAIN_SETTINGS]
```

This lets Platform Pocket remember a separate cursor position for each section.

Example:

- Network can remain on `Wi-Fi Info`
- Docker can remain on `Compose Cheatsheet`
- Settings can remain on `Theme`

Switching between sections does not force every submenu back to item zero.

---

## 6. The UI layer

The UI is built from small reusable drawing functions.

Important examples include:

```cpp
drawHeader(...)
drawStatusHeader()
drawFooter(...)
drawMainMenu()
drawSectionMenu()
drawToolPage()
drawWifiResults()
drawTerminal()
```

This is preferable to placing raw `Display.print()` calls everywhere because shared visual rules stay centralized.

If the header height changes later, `drawHeader()` should be the main place that needs editing.

---

## 7. The tool router

A menu selection does not directly contain all of a feature's code.

Instead, `executeSelectedTool()` acts as a router.

Conceptually:

```text
selectedMainItem
      +
selectedSubItem
      |
      v
executeSelectedTool()
      |
      +--> showWifiInfo()
      +--> scanWifiNetworks()
      +--> showDockerCommands()
      +--> showSystemInfo()
      +--> openTool(...)
```

This keeps navigation code separate from feature code.

That separation becomes increasingly valuable as the project grows.

---

## 8. Generic versus custom screens

Not every feature needs its own state.

Simple information pages can use the generic tool screen:

```cpp
openTool("SYSTEM INFO", text);
```

That stores the title/message and moves to `SCREEN_TOOL`.

Interactive features need dedicated states.

Examples:

- Wi-Fi results need cursor movement -> `SCREEN_WIFI_SCAN`
- Terminal needs keyboard text entry -> `SCREEN_TERMINAL`
- Brightness needs live adjustment -> `SCREEN_BRIGHTNESS`

A good rule is:

> If a page only displays information, reuse `SCREEN_TOOL`. If it has its own interaction model, give it its own state.

---

## 9. Wi-Fi architecture

Wi-Fi scanning demonstrates the full pattern well.

State variables:

```cpp
int wifiNetworkCount;
int selectedWifiNetwork;
int wifiScrollOffset;
```

Work function:

```cpp
scanWifiNetworks();
```

Rendering function:

```cpp
drawWifiResults();
```

Details state:

```cpp
SCREEN_WIFI_DETAILS
```

The feature therefore has separate pieces for:

- stored state
- radio work
- list rendering
- detail rendering
- navigation

That is a miniature version of the architecture used by much larger applications.

---

## 10. Terminal architecture

The terminal has its own state because printable characters, Enter, Delete, and Escape all mean something different there.

Important terminal state includes:

```cpp
String terminalInput;
String terminalLines[6];
int terminalLineCount;
String lastTerminalCommand;
```

The flow is:

```text
Keyboard event
     |
     +--> printable character -> terminalInput
     +--> backspace -> remove character
     +--> Enter -> executeTerminalCommand()
     +--> Escape -> return to main UI

executeTerminalCommand()
     |
     +--> parse command
     +--> perform work
     +--> append output lines
     +--> redraw terminal
```

The local terminal is a command interpreter, not a Linux shell. A future SSH transport can connect the same physical keyboard/display experience to a remote Linux shell.

---

## 11. Themes

The theme is deliberately represented as shared color variables:

```cpp
uint16_t uiBg;
uint16_t uiHeader;
uint16_t uiPanel;
uint16_t uiAccent;
uint16_t uiText;
uint16_t uiMuted;
```

`applyTheme()` changes those values.

The drawing code then uses semantic names such as `uiAccent` instead of hardcoded colors.

That means one theme change affects the entire app consistently.

---

## 12. The full request path

Here is the whole architecture in one example.

The user opens Docker Commands:

```text
Keyboard Enter
      |
      v
loop()
      |
currentScreen == SCREEN_MAIN
      |
selectedMainItem = MAIN_DOCKER
      |
SCREEN_SECTION_MENU
      |
select "Docker Commands"
      |
executeSelectedTool()
      |
showDockerCommands()
      |
openTool(title, text)
      |
SCREEN_TOOL
      |
drawToolPage()
      |
      v
240x135 display
```

That chain — input, state, routing, work, rendering — is the core of Platform Pocket.
