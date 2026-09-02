# Terminal

Platform Pocket v0.5 includes a real local command console. It accepts text from the Cardputer keyboard, parses commands, runs firmware functions, and prints results back into a small terminal buffer.

It is important to understand what it is — and what it is not.

## 1. Local shell versus Linux shell

The ESP32-S3 is running Platform Pocket firmware directly. It is not booting Linux.

So this:

```text
pocket> sysinfo
```

is handled by Platform Pocket's own C++ code.

It is not running `/bin/bash`, PowerShell, or a Linux executable.

A future SSH mode can connect the Cardputer to a remote Linux host. At that point, the physical device can behave like a tiny remote terminal even though Linux is running somewhere else.

---

## 2. Terminal state

The terminal needs to remember more information than a normal menu.

Important state includes:

```cpp
String terminalInput = "";
String terminalLines[6];
int terminalLineCount = 0;
String lastTerminalCommand = "";
```

### `terminalInput`

What the user is currently typing.

```text
pocket> sysinf_
```

### `terminalLines`

A small display-history buffer.

The screen cannot show unlimited output, and embedded RAM is finite, so Platform Pocket keeps only a small number of visible lines.

### `terminalLineCount`

Tracks how many lines are currently valid.

### `lastTerminalCommand`

Stores the most recently executed command for simple history behavior.

---

## 3. Keyboard-to-command flow

The terminal owns input while:

```cpp
currentScreen == SCREEN_TERMINAL
```

The flow is:

```text
M5Cardputer.update()
        |
keyboard changed?
        |
        v
keysState()
        |
        +--> status.word
        |       |
        |       +--> append printable characters
        |
        +--> backspace/delete
        |       |
        |       +--> remove last character
        |
        +--> Enter
        |       |
        |       +--> execute command
        |
        +--> Escape
                |
                +--> leave terminal
```

This is a miniature REPL.

REPL means:

```text
Read
Evaluate
Print
Loop
```

Platform Pocket reads a command, evaluates it, prints output, and waits for another command.

---

## 4. Command parsing

The terminal command function receives a `String`.

A simple command parser usually follows this pattern:

```cpp
String command = terminalInput;
command.trim();

if (command == "help")
{
    // help output
}
else if (command == "wifi")
{
    // Wi-Fi output
}
else if (command.startsWith("echo "))
{
    // use the text after "echo "
}
```

There are two useful command shapes here.

### Exact commands

```text
help
wifi
ip
uptime
```

These are easy to compare with `==`.

### Commands with arguments

```text
echo hello world
ssh server-name
```

These need parsing after a prefix or separation into command + arguments.

---

## 5. Why commands call normal firmware logic

The terminal should not become a second copy of every feature.

For example, if the UI already has code that retrieves system information, the terminal should reuse the same underlying idea rather than implementing a completely separate system-info engine.

A healthy direction is:

```text
             +--> graphical screen
feature logic|
             +--> terminal command
```

rather than:

```text
graphical feature logic
terminal duplicate logic
```

As the project grows, moving shared behavior into small helper functions will make this reuse even cleaner.

---

## 6. Terminal rendering

The display has very little vertical space.

The terminal therefore uses a bounded output area rather than an unlimited scrollback buffer.

Conceptually:

```text
+--------------------------------------+
| TERMINAL                       LOCAL |
+--------------------------------------+
| output line                          |
| output line                          |
| output line                          |
| output line                          |
|                                      |
| pocket> current input_               |
+--------------------------------------+
```

When new output arrives, older lines can be shifted out.

That pattern is often called a ring buffer or rolling buffer, although the current implementation can stay simpler while the output is tiny.

---

## 7. Existing local commands

Current commands include:

| Command | Purpose |
|---|---|
| `help` | Show command list |
| `clear` / `cls` | Clear terminal output |
| `wifi` | Show Wi-Fi status and signal |
| `scan` | Perform a nearby Wi-Fi scan |
| `ip` | Show local network addressing |
| `sysinfo` / `free` | ESP32 CPU, memory, and flash information |
| `uptime` | Time since the device booted |
| `docker` | Docker/Compose quick reference |
| `history` | Show recent command information |
| `version` | Show Platform Pocket version |
| `echo TEXT` | Print supplied text |
| `ssh` | Placeholder for future remote transport |

---

## 8. Adding a command

Suppose you want:

```text
pocket> hostname
```

The simplest implementation is another parser branch:

```cpp
else if (command == "hostname")
{
    terminalPrint("platform-pocket");
}
```

Then add `hostname` to the help text.

For a command with arguments:

```text
pocket> echo hello
```

use something like:

```cpp
else if (command.startsWith("echo "))
{
    String value = command.substring(5);
    terminalPrint(value);
}
```

The `5` skips the characters in `"echo "`.

---

## 9. Future SSH architecture

M5Stack publishes a Cardputer SSH example using LibSSH-ESP32, so remote shell support is technically realistic.

A future design could separate terminal *presentation* from terminal *transport*:

```text
Cardputer keyboard
        |
        v
Terminal UI
        |
        +--> LOCAL transport
        |       |
        |       +--> Platform Pocket command parser
        |
        +--> SSH transport
                |
                +--> remote Linux host
```

That separation would let the same screen and keyboard work with two very different command engines.

Possible future states:

```cpp
SCREEN_TERMINAL_LOCAL
SCREEN_TERMINAL_SSH
SCREEN_SSH_CONNECT
```

Possible saved-host data:

```text
name
host/IP
port
username
```

Passwords should not be casually hardcoded into source control.

---

## 10. A useful next refactor

As the terminal grows, consider introducing a command table instead of a very long `if / else if` chain.

Conceptually:

```cpp
struct TerminalCommand
{
    const char *name;
    const char *description;
    void (*handler)(const String &args);
};
```

Then `help` could even generate itself from the same command table used for execution.

That is a good future exercise because it teaches:

- structs
- function pointers
- table-driven design
- reducing duplicated metadata

For v0.5, the simpler parser is easier to understand and debug.
