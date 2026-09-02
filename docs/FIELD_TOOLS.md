# Platform Pocket Field Tools

Platform Pocket v0.6 turns the Cardputer ADV terminal into a compact field-diagnostics console. These tools are intentionally local, defensive, and operator-driven.

## Terminal commands

- `net` — show the active IPv4 address, gateway, subnet mask, and DNS server.
- `dns HOST` — resolve a hostname using the connected Wi-Fi network.
- `port HOST PORT` — make a short TCP connection attempt to a host/port you own or administer.
- `sha256 TEXT` — calculate a SHA-256 digest locally on the ESP32-S3.
- `cidr PREFIX` — calculate a dotted-decimal subnet mask and address count for CIDR prefixes 0–32.
- `base DECIMAL` — convert a decimal number to hexadecimal and binary.
- `diag` — show Wi-Fi state, free heap, minimum free heap, and uptime.
- `scan`, `wifi`, `ip`, `sysinfo`, `uptime`, `docker`, `history`, and `version` remain available.

## UI upgrades

The Network menu now exposes a real connected-network signal snapshot and live network configuration instead of placeholder pages. Security includes real SHA-256 and defensive TCP reachability instructions. Tools exposes working CIDR and base-conversion helpers through the terminal. Settings → Storage reports flash, sketch, and heap health.

## Build verification

Pull-request CI now compiles the actual `cardputer-adv` firmware before running native unit tests, so embedded-only compile failures cannot hide behind a green host test.

## Safety boundary

The port checker is a single-target TCP reachability diagnostic, not a network scanner. Use it only for systems you own or are authorized to administer. Wi-Fi observations remain passive and do not claim to identify attackers or malicious behavior.
