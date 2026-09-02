# Platform Pocket Feature Guide

Platform Pocket is a handheld ESP32-S3 toolkit built for quick networking, security, platform-engineering, and troubleshooting tasks on the M5Stack Cardputer ADV.

This page explains every user-facing feature in the current menu: **what it is, what it does, why it exists, and whether it is available today**.

> **Status key:** ✅ Available in v0.6 · 🧭 Planned / placeholder

---

## Network

### Wi-Fi Scanner — ✅ Available

**What it is:** A passive scanner for nearby Wi-Fi access points.

**What it does:** Scans for nearby SSIDs and displays signal strength (RSSI), channel information, and whether a network is open or protected. You can move through the scan results and open a network-details view.

**Why it matters:** It gives you a fast picture of the wireless environment without needing a laptop. It is useful when checking coverage, comparing access-point signal strength, finding congested areas, or confirming that a network is visible from a particular location.

**Safety:** The scanner is observational. It does not authenticate to networks, attack access points, or claim that a network is malicious.

### Wi-Fi Info — ✅ Available

**What it is:** A summary of the device's current Wi-Fi connection.

**What it does:** Shows information about the network Platform Pocket is currently connected to, including connection state and useful network details.

**Why it matters:** When troubleshooting connectivity, the first question is often simply, “Am I actually connected, and to what?” This provides that answer directly on the device.

### Signal Monitor — ✅ Available

**What it is:** A live snapshot of the currently connected Wi-Fi signal.

**What it does:** Shows the connected SSID, RSSI in dBm, a human-readable signal rating, Wi-Fi channel, and the device's current IP address.

**Why it matters:** RSSI is one of the fastest ways to understand weak connectivity. The monitor helps you walk around a room, rack, office, or lab and see whether poor placement or weak coverage may be contributing to a problem.

### DNS Lookup — ✅ Available

**What it is:** An on-device hostname resolver.

**What it does:** Resolves a hostname to an IP address from the Terminal using:

```text
dns example.com
```

The device must have an active Wi-Fi connection.

**Why it matters:** DNS failures can look like general network failures. Being able to test name resolution independently helps distinguish “the network is down” from “DNS is broken.”

### Network Tools — ✅ Available

**What it is:** A compact network-configuration dashboard.

**What it does:** Shows the device's current IPv4 address, gateway, subnet mask, DNS server, and MAC address.

**Why it matters:** These are the core values needed to diagnose DHCP, gateway, subnet, and DNS problems. Instead of opening several menus or carrying another machine, you can see them from one screen.

---

## Security

### Security Dashboard — ✅ Available

**What it is:** A high-level view of security-relevant observations available to Platform Pocket.

**What it does:** Surfaces defensive information gathered from the device's local environment and supporting tools.

**Why it matters:** Security troubleshooting often begins with context rather than exploitation. The dashboard gives the operator a compact starting point for understanding the device and nearby network environment.

### Wi-Fi Observations — ✅ Available

**What it is:** A passive wireless-environment checklist.

**What it does:** Highlights observable conditions such as open networks, hidden SSIDs, duplicate SSIDs, and signal strength.

**Why it matters:** These observations can help an administrator notice configuration issues worth investigating. Duplicate names or open networks, for example, may deserve a closer look even though they are not proof of malicious activity.

**Safety:** Platform Pocket deliberately avoids labeling an access point as an attacker or making unsupported threat claims.

### Device Info — ✅ Available

**What it is:** A hardware/device information page.

**What it does:** Displays useful information about the Cardputer/ESP32-S3 environment.

**Why it matters:** Hardware identity and runtime details are useful when debugging firmware, confirming the target device, comparing devices, or reporting an issue.

### Hash Tool — ✅ Available

**What it is:** A local SHA-256 hashing utility.

**What it does:** Calculates the SHA-256 digest of text entered in the Terminal:

```text
sha256 hello world
```

The calculation is performed locally on the ESP32-S3.

**Why it matters:** Hashes are useful for integrity checks, comparing values, verifying that text has not changed, and learning how common security primitives work. Because this runs locally, the text does not need to be sent to an online hashing service.

### Port Check — ✅ Available

**What it is:** A single-target TCP reachability test.

**What it does:** Attempts a short TCP connection to one host and one port:

```text
port 192.168.1.10 443
```

It reports whether the target accepted the connection or did not respond/appeared closed.

**Why it matters:** This helps answer a very practical troubleshooting question: “Can I reach this service from here?” It is useful for checking an SSH server, web service, development box, homelab service, or other system you administer.

**Safety:** This is intentionally a single-target diagnostic, not a port scanner. Use it only on systems you own or are authorized to administer.

---

## Docker

### Docker Commands — ✅ Available

**What it is:** A pocket reference for commonly used Docker commands.

**What it does:** Provides quick command reminders directly on the Cardputer.

**Why it matters:** When working away from your normal workstation, remembering exact container-management syntax can slow you down. A local cheat sheet keeps common operations close at hand.

### Container Cheatsheet — ✅ Available

**What it is:** A reference focused on day-to-day container operations.

**What it does:** Shows useful container-oriented command patterns for inspecting and managing running workloads.

**Why it matters:** Containers are a major part of modern platform engineering. This gives Platform Pocket value even when it is being used as an offline reference instead of an active network diagnostic tool.

### Compose Cheatsheet — ✅ Available

**What it is:** A Docker Compose reference.

**What it does:** Provides frequently used Compose commands and patterns.

**Why it matters:** Compose is common in local development, labs, demos, and smaller deployments. Keeping the commands available offline makes the device a compact operations reference.

### Remote Host — 🧭 Planned

**What it is:** The planned bridge between Platform Pocket and a remote Linux host.

**What it will do:** The goal is to let the handheld connect to an authorized remote system so the Cardputer can act as a small operations terminal rather than only a local reference device.

**Why it matters:** This is one of the features that could turn Platform Pocket into a genuine pocket platform-engineering workstation: diagnose locally, then connect to the system that needs attention.

---

## Terminal

### Local Terminal — ✅ Available

**What it is:** Platform Pocket's keyboard-driven command console.

**What it does:** Accepts local commands directly from the Cardputer keyboard and provides access to networking, diagnostics, conversion, hashing, and reference utilities.

**Why it matters:** A menu is great for discovery, but a shell is faster once you know what you want. The Terminal gives experienced users a direct path to tools without navigating multiple screens.

### `help` / `?` — ✅ Available

**What it does:** Lists available Terminal commands.

**Why it matters:** The device can teach you its own command surface without requiring external documentation.

### `clear` / `cls` — ✅ Available

**What it does:** Clears the Terminal display.

**Why it matters:** Small screens get crowded quickly. Clearing the console keeps active troubleshooting readable.

### `scan` — ✅ Available

**What it does:** Starts a Wi-Fi scan.

**Why it matters:** Provides a fast keyboard shortcut to one of the device's most useful field tools.

### `wifi` — ✅ Available

**What it does:** Shows Wi-Fi-related connection information.

**Why it matters:** Lets you quickly check wireless state without leaving the shell.

### `ip` — ✅ Available

**What it does:** Shows the current IP address and gateway.

**Why it matters:** These are the fastest values to inspect when basic IPv4 connectivity is in question.

### `net` — ✅ Available

**What it does:** Shows IP address, gateway, subnet mask, and DNS server.

**Why it matters:** It provides a more complete network snapshot than `ip`, which is useful when debugging DHCP or routing configuration.

### `dns HOST` — ✅ Available

**What it does:** Resolves a hostname using the active network connection.

**Why it matters:** Separates DNS problems from broader connectivity problems.

### `port HOST PORT` — ✅ Available

**What it does:** Performs a single TCP reachability check to a specified service.

**Why it matters:** Helps determine whether a remote service is reachable from the device's current network location.

### `sha256 TEXT` — ✅ Available

**What it does:** Generates a SHA-256 digest locally.

**Why it matters:** Useful for integrity checks and security learning without relying on a web service.

### `cidr PREFIX` — ✅ Available

**What it does:** Converts a CIDR prefix such as `/24` or `/27` into its dotted-decimal subnet mask and address count.

Example:

```text
cidr 24
```

**Why it matters:** Subnet calculations come up constantly in networking and cloud/platform work. This turns the Cardputer into a quick subnet reference instead of making you mentally reconstruct masks.

### `base DECIMAL` — ✅ Available

**What it does:** Converts a decimal value to hexadecimal and binary.

Example:

```text
base 255
```

**Why it matters:** Base conversion is useful in networking, embedded development, bitmask work, permissions, and low-level debugging.

### `diag` — ✅ Available

**What it does:** Displays Wi-Fi state, free heap, minimum observed heap, and uptime.

**Why it matters:** This is a quick health check for both connectivity and firmware memory pressure. It is particularly useful when tracking down instability on an embedded device.

### `sysinfo` / `free` — ✅ Available

**What it does:** Displays system/runtime information such as CPU and memory data.

**Why it matters:** Gives developers and operators a quick view of the environment when debugging performance or resource issues.

### `uptime` — ✅ Available

**What it does:** Shows how long the current firmware session has been running.

**Why it matters:** Uptime is useful when investigating unexpected resets or determining whether a device has remained stable over time.

### `docker` — ✅ Available

**What it does:** Surfaces Docker command/reference information from the Terminal.

**Why it matters:** Keeps commonly used platform-engineering knowledge one command away.

### `history` — ✅ Available

**What it does:** Shows the previous Terminal command.

**Why it matters:** On a small keyboard, re-entering commands is annoying. Even lightweight history improves usability. v0.6 also fixes the earlier behavior where `history` could report itself instead of the prior command.

### `version` — ✅ Available

**What it does:** Shows the Platform Pocket firmware version.

**Why it matters:** Version information is essential when debugging, comparing devices, or confirming which feature set should be available.

---

## Notes

### View Notes — 🧭 Planned

**What it is:** A planned reader for notes saved on removable storage.

**What it will do:** Display notes stored on the device/SD card.

**Why it matters:** A field device is more useful when it can carry runbooks, IP plans, incident notes, command snippets, and troubleshooting checklists without needing a network connection.

### New Note — 🧭 Planned

**What it is:** A planned keyboard-driven note editor.

**What it will do:** Let the user create notes directly from the Cardputer keyboard.

**Why it matters:** This would allow Platform Pocket to capture information during an incident, site visit, lab session, or troubleshooting task instead of being read-only.

### Delete Note — 🧭 Planned

**What it is:** Planned note-file management.

**What it will do:** Remove saved notes from local/SD storage.

**Why it matters:** Notes need lifecycle management if the device is going to function as a real portable notebook rather than a static reference viewer.

### SD Storage — 🧭 Planned

**What it is:** Planned SD-card status and file information.

**What it will do:** Show capacity, available space, and note/file information stored on the SD card.

**Why it matters:** Once notes and other persistent data use removable storage, the operator needs visibility into whether the card is mounted and how much space remains.

---

## Tools

### IP Tools — ✅ Available

**What it is:** A shortcut into the device's IP/network troubleshooting capabilities.

**What it does:** Shows live network configuration when connected and points to Terminal networking commands when more detail is needed.

**Why it matters:** IP configuration is at the center of most connectivity troubleshooting. This gives both menu-driven and command-driven access to the same information.

### Subnet Helper — ✅ Available

**What it is:** A CIDR/subnet calculation helper.

**What it does:** Uses the Terminal `cidr` command to calculate subnet masks and address counts.

**Why it matters:** It eliminates repetitive subnet-mask lookup and makes Platform Pocket useful as a networking study and field-reference tool.

### Base Converter — ✅ Available

**What it is:** A decimal/hexadecimal/binary conversion utility.

**What it does:** Uses the Terminal `base` command to convert decimal input into hexadecimal and binary forms.

**Why it matters:** Developers and network engineers regularly move between number bases. A local converter is especially handy on an embedded keyboard device where pulling out another calculator defeats the point.

### Password Generator — ✅ Available

**What it is:** An on-device random password generator.

**What it does:** Generates a 16-character password from an intentionally readable character set that excludes some easily confused characters.

**Why it matters:** It provides a quick local credential-generation utility and demonstrates use of the ESP32's randomness source without depending on an online generator.

### System Info — ✅ Available

**What it is:** A runtime system-information page.

**What it does:** Shows CPU frequency, free heap, flash size, uptime, and ESP SDK version.

**Why it matters:** These values make firmware debugging much easier and help confirm whether memory or runtime state may be contributing to a problem.

---

## Settings

### Brightness — ✅ Available

**What it is:** Display brightness control.

**What it does:** Lets the user adjust the Cardputer screen brightness.

**Why it matters:** A handheld device may be used in bright rooms, dark rooms, server closets, or battery-conscious situations. Brightness control improves readability and power management.

### Theme — ✅ Available

**What it is:** A selectable interface color theme.

**What it does:** Changes Platform Pocket's display palette. Current firmware includes multiple themes such as Midnight Cyan, Matrix Green, and Amber Ops.

**Why it matters:** Beyond appearance, different contrast levels can improve readability in different lighting conditions and make a small embedded interface more pleasant to use.

### Wi-Fi Settings — 🧭 Planned

**What it is:** Planned management for trusted/saved Wi-Fi profiles.

**What it will do:** Provide a dedicated place to manage connection configuration instead of relying only on transient runtime Wi-Fi state.

**Why it matters:** Persistent profiles are necessary if Platform Pocket is going to become a device you can routinely carry between known environments without reconfiguring connectivity each time.

### Storage — ✅ Available

**What it is:** Internal flash and memory-health information.

**What it does:** Shows flash size, sketch size, remaining sketch space, free heap, and minimum free heap.

**Why it matters:** Embedded systems have tight resource limits. This page makes those limits visible so firmware growth, memory pressure, and available update space can be monitored directly on the device.

### About — ✅ Available

**What it is:** Product/version information.

**What it does:** Shows the Platform Pocket version and summarizes the device as an ESP32-S3 handheld toolkit for networking, platform work, and diagnostics.

**Why it matters:** It gives users an immediate way to identify what they are running and provides useful context when sharing screenshots, reporting bugs, or comparing firmware builds.

---

## Reliability and engineering features

These are not menu items, but they directly affect the quality of Platform Pocket.

### Wi-Fi scan error handling — ✅ Available

**What it does:** Negative Wi-Fi scan errors are normalized into a clean zero-results state instead of being treated like a valid network count.

**Why it matters:** Hardware APIs fail sometimes. Graceful handling keeps the UI predictable and prevents bad scan values from leaking into navigation logic.

### Cardputer firmware compile gate — ✅ Available

**What it does:** CI now runs the real embedded firmware build:

```text
pio run -e cardputer-adv
```

before running native tests.

**Why it matters:** Native tests can pass even when device-specific code no longer compiles. Building the actual ESP32-S3 target in CI catches firmware-level regressions before they reach `main`.

### Native tests — ✅ Available

**What it does:** Runs host-side PlatformIO tests with:

```text
pio test -e native
```

**Why it matters:** Fast native tests are useful for validating logic without flashing physical hardware every time. Used together with the real firmware build, they provide a much stronger safety net.

---

## The direction of Platform Pocket

The goal is not to cram every possible utility onto a tiny screen. Platform Pocket is meant to become a **focused handheld platform-engineering workstation**:

1. Observe the local environment.
2. Diagnose connectivity and device health.
3. Carry useful offline platform references.
4. Perform small calculations and integrity checks locally.
5. Eventually connect to authorized remote systems for deeper operations work.

Features should earn their place by making one of those jobs meaningfully faster or easier on a pocket-sized device.
