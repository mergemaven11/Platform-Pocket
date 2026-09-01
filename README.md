# ⚡ Platform Pocket

> A pocket-sized ESP32-S3 platform engineering toolkit for networking, security diagnostics, Docker reference tools, notes, and system utilities — built on the M5Stack Cardputer ADV.

Platform Pocket turns the Cardputer ADV into a tiny terminal-style companion for learning and practicing **networking, embedded C++, platform engineering, security diagnostics, and systems thinking**.

The project is intentionally built as both a **real tool** and a **learning project**: the code is heavily commented, features are added in architectural layers, and each section is designed to teach the ideas behind it — not just make the screen do something cool.

<p align="center">
  <img src="assets/platform-pocket-v0.5.png" alt="Platform Pocket v0.4 project overview" width="100%">
</p>

---

## 🧭 Main Menu

```text
PLATFORM POCKET

Network
 ├─ Wi-Fi Scanner
 ├─ Wi-Fi Info
 ├─ Signal Monitor
 ├─ DNS Lookup
 └─ Network Tools

Security
 ├─ Security Dashboard
 ├─ Wi-Fi Observations
 ├─ Device Info
 ├─ Hash Tool
 └─ Port Check

Docker
 ├─ Docker Commands
 ├─ Container Cheatsheet
 ├─ Compose Cheatsheet
 └─ Remote Host      [later]

Notes
 ├─ View Notes
 ├─ New Note
 ├─ Delete Note
 └─ SD Storage

Quick Tools
 ├─ IP Tools
 ├─ Subnet Helper
 ├─ Base Converter
 ├─ Password Generator
 └─ System Info

Settings
 ├─ Brightness
 ├─ Theme
 ├─ Wi-Fi Settings
 ├─ Storage
 └─ About
```

---

## ✅ Current Features

### Network
- Nearby Wi-Fi scanning
- Signal strength (RSSI)
- Channel display
- Open-network detection
- Duplicate SSID observations
- Per-network detail view
- Local Wi-Fi/MAC information

### Security
- Defensive Wi-Fi observation dashboard
- Open/hidden/duplicate SSID summaries
- Device information
- Security-focused tools planned around diagnostics and user-owned systems

### Docker
- Docker command reference
- Container concepts cheatsheet
- Docker Compose cheatsheet
- Remote-host management planned for later

### Quick Tools
- Password generator
- ESP32/system information
- IP, subnet, and base-conversion tools planned

### Notes + Settings
- Menu architecture is in place
- SD-card notes, keyboard text entry, theme control, storage info, and richer settings are coming next

---

## 🎮 Controls

| Key | Action |
|---|---|
| `;` | Move up |
| `.` | Move down |
| `Enter` | Select / open |
| `Fn + \`` | Back / Escape |

The control scheme uses the physical Cardputer keyboard positions that have proven reliable during development.

---

## 🧠 Built to Teach

Platform Pocket is intentionally documented for learning.

The code explains concepts like:

- C++ arrays
- `enum` values
- `switch` statements
- functions
- references
- state machines
- menu routing
- ESP32 Wi-Fi APIs
- RSSI and wireless channels
- program state
- display rendering
- input handling

The goal is to be able to open the source months later and still understand **what the code is doing and why it was designed that way**.

---

## 🏗️ Architecture

Platform Pocket is moving toward a simple state-machine architecture:

```text
Keyboard Input
      ↓
    loop()
      ↓
Screen State
      ↓
Menu Router
      ↓
Tool Function
      ↓
Display Output
```

Instead of using mystery numbers like:

```cpp
currentScreen = 3;
```

Platform Pocket uses readable states such as:

```cpp
currentScreen = SCREEN_WIFI_DETAILS;
```

That makes the firmware easier to learn, debug, and extend.

---

## 🧰 Hardware

Primary target:

- **M5Stack Cardputer ADV**
- ESP32-S3
- Built-in keyboard
- Built-in display
- Wi-Fi
- SD-card expansion
- GNSS / LoRa-capable expansion hardware

---

## 🛠️ Development Stack

- C++
- Arduino framework
- PlatformIO
- VS Code
- M5Cardputer library
- Git + GitHub
- GitLens for code-history exploration

---

## 🚀 Build

This project uses PlatformIO.

Typical workflow:

```text
Edit code
   ↓
Build
   ↓
Upload
   ↓
Test on Cardputer
   ↓
Commit
   ↓
Push to GitHub
```

The repository keeps generated PlatformIO build files and editor caches out of Git via `.gitignore`.

---

## 🚧 Next Step — SD Card Notes

The next major implementation step is to turn the **Notes** section into a real persistent field notebook.

The goal is for notes to survive reboots and power cycles by storing them on the microSD card instead of keeping them only in RAM.

Planned implementation:

- [ ] Initialize and verify the SD card at boot
- [ ] Create `/platform_pocket/notes/` automatically
- [ ] Add full Cardputer keyboard text entry
- [ ] Save each note as a `.txt` file
- [ ] Load and list existing notes
- [ ] View saved notes on-device
- [ ] Delete selected notes safely
- [ ] Show SD-card capacity and storage status
- [ ] Handle missing or unreadable SD cards gracefully

### Why this is next

Persistent notes are the first feature that connects several important embedded-development concepts at once: **filesystem I/O, keyboard input, state management, error handling, storage, and user-created data**.

Once this works, Platform Pocket becomes much more than a menu demo — it becomes a device that can actually keep commands, troubleshooting findings, configs, reminders, and field notes with you.

---

## 🗺️ Roadmap

Planned next jumps include:

- **v0.5:** SD-card notes + keyboard text entry
- Signal monitor
- DNS lookup
- Network diagnostics
- SHA-256 hash tool
- Defensive port checker for user-owned/administered hosts
- Subnet calculator
- Base converter
- Theme switching
- Interactive brightness controls
- Wi-Fi profile management
- Storage dashboard
- Remote Docker host support
- GNSS/LoRa integration experiments
- Better internal docs under `docs/`

---

## 🛡️ Security Philosophy

Platform Pocket is intended for **defensive diagnostics, learning, and systems you own or administer**.

Wireless observations such as duplicate SSIDs, hidden networks, or signal changes are treated as **informational indicators**, not proof of malicious activity.

The project is not intended to include disruptive wireless attacks, credential theft, jamming, or unauthorized access tooling.

---

## 📚 Project Goal

The bigger idea behind Platform Pocket is simple:

> Carry a tiny platform-engineering lab in your pocket.

Something you can use to inspect a network, review commands, learn C++, experiment with ESP32 hardware, take notes, and gradually turn into a genuinely useful field companion.

---

## 📌 Status

**Early development — v0.x**

Core menu architecture and the first real networking/security utilities are being built now.

Expect frequent changes while the project grows. ⚙️📡

---

## 🤝 Contributions

This is currently a personal learning/build project, but ideas, issues, and thoughtful suggestions are welcome.

---

## 📄 License

No license has been selected yet.
