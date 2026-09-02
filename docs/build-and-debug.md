# Build and Debug

This guide connects the source code to the actual Cardputer ADV in your hand.

## 1. What PlatformIO does

PlatformIO is the build system and device workflow for this project.

It reads `platformio.ini`, downloads the required framework/libraries, compiles the C++ firmware, links it for the ESP32-S3 target, and can upload the resulting binary to the device.

The basic flow is:

```text
src/main.cpp
    |
    v
PlatformIO
    |
    +--> Arduino framework
    +--> M5Cardputer library
    +--> ESP32 toolchain
    |
    v
firmware binary
    |
    v
USB upload
    |
    v
Cardputer ADV
```

---

## 2. The project configuration

The project currently uses an environment similar to:

```ini
[env:cardputer-adv]
platform = espressif32
board = esp32-s3-devkitc-1
framework = arduino
```

The important ideas are:

- `platform` selects the ESP32 PlatformIO ecosystem
- `board` chooses the ESP32-S3 hardware definition used by the toolchain
- `framework` tells PlatformIO this is Arduino-style firmware
- `lib_deps` pulls in M5Cardputer

USB-related build flags enable the ESP32-S3 USB CDC behavior used during development.

---

## 3. Build without uploading

From the repository root:

```bash
pio run
```

This is the first command to run after code changes.

A successful build tells you that the compiler and linker accepted the firmware for the configured target.

It does **not** prove that the UI fits the physical display or that hardware behavior is correct.

---

## 4. Upload

With the Cardputer connected over USB:

```bash
pio run --target upload
```

If `upload_port` is explicitly configured in `platformio.ini`, PlatformIO will try that port.

On Windows, this may look like:

```text
COM3
COM4
COM7
```

The exact number can change when hardware or USB configuration changes.

If upload fails because the configured COM port no longer exists, check Windows Device Manager or run PlatformIO's device listing.

---

## 5. Serial monitor

The serial monitor is useful for messages that do not fit on the Cardputer display.

```bash
pio device monitor
```

The project currently uses:

```ini
monitor_speed = 115200
```

If you add debugging output:

```cpp
Serial.begin(115200);
Serial.println("Starting Wi-Fi scan");
```

then the serial monitor can show it.

A common embedded-debugging pattern is:

```text
Display = user-facing information
Serial  = developer-facing information
```

---

## 6. Compile errors versus runtime bugs

These are different categories.

### Compile error

Example:

```text
'foo' was not declared in this scope
```

The firmware cannot be built. Fix the source before uploading.

### Link error

Example:

```text
undefined reference to ...
```

A function was declared or referenced but the final program cannot find its implementation/library symbol.

### Runtime bug

The firmware builds and uploads, but behavior is wrong.

Examples:

- selection skips two rows
- text overlaps the footer
- Wi-Fi results do not scroll correctly
- terminal Backspace removes the wrong character

These require testing the actual running firmware.

---

## 7. Why hardware validation matters

A syntax check can prove C++ structure is valid, but only the physical Cardputer can prove things like:

- font readability
- exact 240×135 spacing
- keyboard feel and repeat behavior
- brightness range
- Wi-Fi radio behavior
- USB upload behavior
- visual contrast under real lighting

For UI work, treat the device itself as part of the test environment.

---

## 8. A safe development loop

A good workflow for this repo is:

```text
1. Create/use a feature branch
2. Make one understandable change
3. pio run
4. Fix compiler warnings/errors
5. Upload to Cardputer
6. Test every key path touched
7. Check Back/Escape behavior
8. Check screen edges for clipping
9. Commit
10. Open/update PR
```

For an interactive feature, test both the happy path and the exit path.

Example for Terminal:

```text
open terminal
-> type command
-> use backspace
-> execute command
-> clear terminal
-> enter unknown command
-> escape back out
```

---

## 9. Debugging the state machine

When navigation behaves strangely, inspect these three values first:

```cpp
currentScreen
selectedMainItem
selectedSubItem[selectedMainItem]
```

Temporary serial logging can make state transitions obvious:

```cpp
Serial.print("screen=");
Serial.println((int)currentScreen);
```

Because enums are integers underneath, casting to `int` is a quick debugging trick.

If Enter appears to do the wrong thing, trace:

```text
Which screen am I in?
Which menu item is selected?
Which router branch executes?
Which state does that function set next?
Which draw function runs?
```

That sequence finds a large percentage of navigation bugs.

---

## 10. Debugging display problems

If something is clipped or overlaps:

1. identify the y-coordinate of the header bottom
2. identify the y-coordinate of the footer top
3. calculate the usable content height
4. count row height × number of rows
5. shorten labels or reduce visible rows if necessary

For the current design, the footer begins around y=120, so content should remain above that boundary.

Long dynamic text such as SSIDs should use truncation rather than assuming it will fit.

---

## 11. Debugging keyboard problems

When a key repeats too quickly, do not immediately add large delays everywhere.

First determine whether the code is reacting to:

- a physical key being held
- a keyboard state change
- a newly generated printable character

The M5Cardputer library provides change/press state so input can be event-driven.

Large delays can make the whole UI feel unresponsive and block unrelated work.

---

## 12. When to use `delay()`

Small delays can be acceptable for simple device interactions, but they stop the Arduino loop from doing other work.

This:

```cpp
delay(1000);
```

means the firmware is effectively asleep for one second.

For future live features such as signal graphs, timers, SSH traffic, or background status updates, prefer time checks:

```cpp
if (millis() - lastUpdate >= 500)
{
    lastUpdate = millis();
    // perform periodic work
}
```

This style is called non-blocking timing.

---

## 13. Before merging a hardware feature

Use this checklist:

- [ ] `pio run` succeeds
- [ ] firmware uploads successfully
- [ ] startup reaches the main menu
- [ ] navigation works in both directions
- [ ] Enter opens the expected feature
- [ ] Escape returns to the expected screen
- [ ] no text overlaps header/footer
- [ ] no obvious display flicker
- [ ] terminal typing works if terminal code changed
- [ ] Wi-Fi scanning works if radio code changed
- [ ] README/docs match the actual behavior

Embedded code is finished only when both the compiler **and the hardware** agree with you.
