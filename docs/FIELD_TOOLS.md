# Platform Pocket Field Tools

Platform Pocket v0.6 turns the Cardputer ADV into a compact field-diagnostics console. The tools are intentionally local, defensive, and operator-driven.

For the complete product catalog, see [FEATURES.md](FEATURES.md).

## `net`

**What it does:** Shows the active IPv4 address, gateway, subnet mask, and DNS server.

**Why it matters:** These values quickly reveal whether DHCP supplied a sensible configuration and whether routing or DNS may be the source of a connectivity problem.

## `dns HOST`

**What it does:** Resolves a hostname using the current Wi-Fi connection.

```text
dns example.com
```

**Why it matters:** DNS failures often look like general network failures. Testing resolution separately helps identify whether basic connectivity works while name resolution does not.

## `port HOST PORT`

**What it does:** Makes a short TCP connection attempt to one host and one port.

```text
port 192.168.1.10 443
```

**Why it matters:** It answers a practical field question: “Can this device reach that service from this network?” That is useful for checking an authorized SSH server, web service, homelab host, or development system.

**Safety:** This is a single-target reachability diagnostic, not a port scanner. Use it only for systems you own or are authorized to administer.

## `sha256 TEXT`

**What it does:** Calculates a SHA-256 digest locally on the ESP32-S3.

```text
sha256 hello world
```

**Why it matters:** Hashes are useful for integrity checks, comparing values, and learning common security primitives. Running locally also means the input does not need to be sent to an online hashing site.

## `cidr PREFIX`

**What it does:** Converts CIDR prefixes from 0–32 into a dotted-decimal subnet mask and address count.

```text
cidr 24
cidr 27
```

**Why it matters:** Subnet calculations are common in networking, cloud, and platform engineering. A built-in helper removes repetitive mask lookup and makes the device useful as both a field tool and learning aid.

## `base DECIMAL`

**What it does:** Converts a decimal value into hexadecimal and binary.

```text
base 255
```

**Why it matters:** Number-base conversion comes up in networking, embedded development, bitmasks, permissions, and low-level debugging.

## `diag`

**What it does:** Shows Wi-Fi state, free heap, minimum observed heap, and uptime.

**Why it matters:** This combines network state and firmware resource health into a fast sanity check. Low heap or unexpected uptime can help explain instability that otherwise looks like a network problem.

## Existing shell shortcuts

### `scan`

Starts a nearby Wi-Fi scan. Useful when checking coverage, visibility, or nearby access points without navigating through the menu.

### `wifi`

Shows current Wi-Fi information. Useful for quickly confirming wireless state from the shell.

### `ip`

Shows the current IP address and gateway. Useful for the fastest possible IPv4 configuration check.

### `sysinfo` / `free`

Shows runtime/system information. Useful when debugging memory or device-resource problems.

### `uptime`

Shows how long the firmware has been running. Useful when investigating resets or stability.

### `docker`

Opens Docker-oriented reference information. Useful when Platform Pocket is being used as an offline platform-engineering cheat sheet.

### `history`

Shows the previous command. This reduces retyping on the small Cardputer keyboard. v0.6 fixes the earlier behavior where `history` could report itself instead of the preceding command.

### `version`

Shows the installed Platform Pocket version. Useful for bug reports, troubleshooting, and confirming which feature set should be present.

## Menu-level field tools

### Signal Monitor

**What it does:** Shows the connected SSID, RSSI, signal-quality label, channel, and IP address.

**Why it matters:** Weak Wi-Fi is often physical rather than logical. This gives you a quick way to compare signal quality while moving around a room, rack, office, or lab.

### Network Tools

**What it does:** Shows IP address, gateway, subnet mask, DNS server, and MAC address on one page.

**Why it matters:** It gathers the most important network configuration values into one field-friendly view.

### Storage + Memory

**What it does:** Shows flash size, sketch size, remaining sketch space, free heap, and minimum free heap.

**Why it matters:** Embedded firmware has hard resource limits. This page makes those limits visible so memory pressure and firmware growth are easier to diagnose.

## Build verification

Pull-request CI now compiles the actual Cardputer ADV firmware:

```text
pio run -e cardputer-adv
```

and then runs native tests:

```text
pio test -e native
```

**Why it matters:** Host-side tests can pass while device-specific C++ fails to compile. Running both checks prevents an embedded-only build regression from hiding behind a green native test.

## Safety boundary

Platform Pocket's security and network features are designed for defensive diagnostics. Wi-Fi observations are passive and do not claim to identify attackers. The TCP checker is deliberately limited to one target and one port per command.
