# SD Workspace and Persistence

Platform Pocket v0.7 turns the Cardputer ADV microSD slot into a persistent workspace. The feature is designed around large cards such as a 64 GB microSD card, while still allowing the device to boot normally when no card is present.

## Why this exists

Before v0.7, most Platform Pocket tools were session-oriented: they displayed information, but that information disappeared after a reboot. Persistent storage changes the device from a collection of handheld utilities into a field workstation that can keep notes, diagnostics, runbooks, exports, and future incident records.

## Hardware behavior

Platform Pocket uses the Cardputer ADV microSD interface documented by M5Stack: CS GPIO 12, MOSI GPIO 14, SCK GPIO 40, and MISO GPIO 39. The card is initialized at 25 MHz.

If the card is missing, unreadable, or cannot be mounted, Platform Pocket continues to boot. Storage-dependent features report that the workspace is offline instead of blocking the rest of the firmware.

## Workspace layout

On first successful mount, Platform Pocket creates:

```text
/platform-pocket/
├── notes/
├── incidents/
├── diagnostics/
├── logs/
├── runbooks/
├── snippets/
├── profiles/
├── exports/
└── config/
```

Each directory has a purpose so future features can grow without dumping unrelated files into the SD card root.

### `notes/`

**What it does:** Stores operator notes. v0.7 starts with `notes/inbox.md`, a persistent Markdown quick-note inbox.

**Why it matters:** Troubleshooting often produces small observations that are easy to forget. Notes survive reboot and can later be opened on a computer as ordinary Markdown.

### `incidents/`

**What it does:** Reserved for structured incident workspaces.

**Why it matters:** Future incident sessions can keep notes, snapshots, logs, and exported evidence together instead of mixing them with general files.

### `diagnostics/`

**What it does:** Stores diagnostic snapshots such as `snapshot-0001.txt`.

**Why it matters:** A saved snapshot lets you compare device and network state over time instead of relying on memory or screenshots.

### `logs/`

**What it does:** Reserved for persistent device and tool logs.

**Why it matters:** Reboots should not erase the only record of a failure. Future logging can use this directory without changing the storage layout.

### `runbooks/`

**What it does:** Reserved for offline troubleshooting and platform-engineering runbooks.

**Why it matters:** The SD card can hold a large offline reference library without requiring internet access or a paid API.

### `snippets/`

**What it does:** Reserved for reusable shell commands and configuration fragments.

**Why it matters:** Common Docker, Kubernetes, Linux, Git, and networking commands can become a searchable pocket reference.

### `profiles/`

**What it does:** Reserved for non-secret connection and environment profiles.

**Why it matters:** Named environments and hosts can later be referenced consistently without hard-coding them into firmware.

### `exports/`

**What it does:** Reserved for bundles intended to be copied off the SD card.

**Why it matters:** Future incident reports and diagnostic bundles need a predictable place for computer-friendly output.

### `config/`

**What it does:** Reserved for persistent Platform Pocket configuration.

**Why it matters:** Settings can eventually survive firmware updates without being compiled into the binary.

## Live v0.7 storage features

### SD status

Use the Notes → SD Storage page, Settings → Storage, or the Terminal command:

```text
sd
```

**What it does:** Reports the card type, total capacity, used space, free space, and workspace root.

**Why it matters:** You can confirm that the 64 GB card is actually mounted and writable before depending on persistent tools.

### Workspace map

```text
workspace
```

**What it does:** Shows the standard `/platform-pocket/` directory layout.

**Why it matters:** It gives both the operator and future features one predictable filesystem contract.

### Persistent quick notes

```text
note replaced switch cable and DNS recovered
notes
```

**What it does:** `note TEXT` appends a Markdown entry to `/platform-pocket/notes/inbox.md`. `notes` reports the inbox location and size.

**Why it matters:** Notes survive reboot and remain readable outside Platform Pocket. Entries currently use device uptime rather than wall-clock time because v0.7 does not yet depend on an RTC or network time source.

### Diagnostic snapshots

```text
snapshot
```

**What it does:** Saves a numbered text snapshot under `/platform-pocket/diagnostics/`. The snapshot includes uptime, heap health, CPU frequency, Wi-Fi state, and—when connected—SSID, RSSI, channel, IP, gateway, subnet mask, and DNS server.

**Why it matters:** This creates a durable point-in-time record for troubleshooting and lays the groundwork for future snapshot comparison and incident reports.

## What comes next

The storage foundation is intentionally broader than the first v0.7 features. Planned layers include a file manager, note editor, incident notebook, diagnostic snapshot comparison, Wi-Fi survey history, persistent Terminal history, offline runbooks, log viewing, structured JSON/YAML/CSV viewing, checksum manifests, export bundles, and the Pocket Troubleshooter knowledge base.
