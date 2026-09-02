# Platform Pocket Workstation

Platform Pocket v0.8 builds on the v0.7 SD workspace and keeps the local Terminal as a first-class interface. The 64 GB card stores durable content while the ESP32-S3 firmware keeps only small working buffers in RAM.

## Terminal stays

The Terminal is not being removed. It is the fastest interface for experienced users and now acts as a command surface for the SD-backed workstation features.

New commands include:

- `files` — summarize files in notes, incidents, diagnostics, and runbooks.
- `edit` — open the full-screen Markdown editor.
- `incident new TITLE` — create/select an incident notebook.
- `incident add TEXT` — append a timestamped observation to the active incident.
- `incident` — show the active incident.
- `diff` — compare the two newest diagnostic snapshots.
- `runbooks` — list bundled offline runbooks.
- `runbook NAME` — open an offline runbook such as `dns`, `wifi`, `service`, or `memory`.
- `troubleshoot TEXT` — run the offline troubleshooting decision tree.

## File Manager

**What it does:** Counts and summarizes persistent files across the most useful workspace areas without loading entire files into RAM.

**Why it matters:** A 64 GB card can contain far more data than the ESP32-S3 should ever attempt to hold in memory. The file manager treats SD as the durable source of truth and keeps the on-device view lightweight.

## Full-screen Markdown Editor

**What it does:** Provides a dedicated keyboard editing screen backed by `/platform-pocket/notes/editor.md`. The active buffer is intentionally bounded to protect heap health; pressing Enter saves the current buffer and inserts a new line.

**Why it matters:** Platform Pocket can capture real field notes without needing a phone or laptop, while still respecting embedded-memory limits.

## Incident Notebook

**What it does:** Creates Markdown incident files under `/platform-pocket/incidents/`, tracks the active incident, and appends timestamped observations.

**Why it matters:** Diagnostics become more useful when they are attached to an investigation. The incident notebook gives the operator a durable narrative of what was observed and changed.

## Snapshot Compare

**What it does:** Reads the two newest diagnostic snapshots and compares tracked fields including Wi-Fi state, SSID, RSSI, IP, gateway, DNS, and free heap.

**Why it matters:** Instead of eyeballing two files, Platform Pocket can answer “what changed?” directly on the handheld.

## Offline Runbook Library

**What it does:** Seeds small Markdown runbooks for DNS failures, Wi-Fi problems, unreachable services, and device-memory instability. They live on SD and can be read without Internet access.

**Why it matters:** A field troubleshooting device should still be useful when the network itself is broken.

## Pocket Troubleshooter

**What it does:** Maps plain symptom text to an offline checklist and related runbook. Current categories cover DNS/name resolution, Wi-Fi/signal problems, service reachability, and device reset/memory problems.

**Why it matters:** This provides guided troubleshooting without cloud AI, an API key, or an Internet connection. The logic is intentionally transparent and deterministic.

## Storage strategy

The 64 GB microSD card is used for durable content. Firmware flash stores code; RAM stores only the active screen, small strings, and bounded editor/file reads. This separation lets the Terminal and workstation features coexist without trying to treat SD capacity as executable-memory capacity.
