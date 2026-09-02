# Platform Pocket v0.8 Feature Guide

Platform Pocket v0.8 keeps the local Terminal as a first-class interface and adds an SD-backed workstation layer on top of the v0.7 persistence foundation.

This page documents the features introduced or materially expanded in v0.8. For the earlier networking, security, Docker, system, and v0.7 storage features, see [Complete Feature Guide](FEATURES.md).

## Terminal — ✅ Available

**What it is:** The keyboard-driven local command console remains a core Platform Pocket interface.

**What it does:** Runs networking, diagnostics, storage, incident, runbook, file, and troubleshooting commands directly from the Cardputer keyboard.

**Why it matters:** Menus are useful for discovery, but the Terminal is faster for repeat field work and lets the new SD-backed workstation features share one consistent command surface.

### New v0.8 Terminal commands

- `files` — summarize files in notes, incidents, diagnostics, and runbooks.
- `edit` — open the full-screen Markdown editor.
- `incident new TITLE` — create and select an incident notebook.
- `incident add TEXT` — append a timestamped observation to the active incident.
- `incident` — show the active incident.
- `diff` — compare the two newest diagnostic snapshots.
- `runbooks` — list bundled offline runbooks.
- `runbook NAME` — read a runbook such as `dns`, `wifi`, `service`, or `memory`.
- `troubleshoot TEXT` — run the deterministic offline troubleshooting decision tree.

## File Manager — ✅ Available

**What it is:** A lightweight view into the SD-backed Platform Pocket workspace.

**What it does:** Counts and summarizes files across the notes, incidents, diagnostics, and runbooks areas without loading the entire 64 GB card into RAM.

**Why it matters:** The microSD card can hold vastly more data than the ESP32-S3 should ever keep in memory. The File Manager gives useful visibility while preserving heap stability.

## Full-screen Markdown Editor — ✅ Available

**What it is:** A dedicated keyboard editing screen for persistent Markdown notes.

**What it does:** Loads and saves `/platform-pocket/notes/editor.md`. The active working buffer is intentionally bounded to 1 KB so the editor does not consume unbounded RAM.

**Why it matters:** Platform Pocket can capture real field notes without needing a phone or laptop while still respecting embedded-memory limits.

## Incident Notebook — ✅ Available

**What it is:** A persistent investigation log stored under `/platform-pocket/incidents/`.

**What it does:** Creates incident Markdown files, tracks the active incident, and appends timestamped observations from the Terminal.

**Why it matters:** Troubleshooting is more useful when observations, changes, and diagnostic evidence are tied together in one durable record.

## Snapshot Compare — ✅ Available

**What it is:** A before/after comparison tool for saved diagnostic snapshots.

**What it does:** Reads the two newest diagnostic snapshots and compares tracked values including Wi-Fi state, SSID, RSSI, IP address, gateway, DNS, and free heap.

**Why it matters:** Instead of manually opening two files and looking for differences, Platform Pocket can answer “what changed?” directly on the handheld.

## Offline Runbook Library — ✅ Available

**What it is:** A small SD-backed knowledge base for common platform and network problems.

**What it does:** Seeds Markdown runbooks for DNS failures, weak/unreliable Wi-Fi, unreachable services, and memory/reset troubleshooting.

**Why it matters:** A field troubleshooting device should remain useful when Internet access is unavailable or the network itself is the thing that is broken.

## Pocket Troubleshooter — ✅ Available

**What it is:** A deterministic offline symptom-to-checklist engine.

**What it does:** Accepts plain symptom text and maps it to a transparent troubleshooting checklist and related runbook. Current categories cover DNS/name resolution, Wi-Fi/signal problems, service reachability, and device reset/memory problems.

**Why it matters:** It gives guided troubleshooting without cloud AI, an API key, or an Internet connection. The logic is intentionally explainable and predictable.

## Why the 64 GB card does not replace the Terminal

The SD card is durable storage, not extra executable RAM. Platform Pocket v0.8 deliberately splits responsibilities:

- **Firmware flash:** program code.
- **ESP32-S3 RAM:** active screen state and small bounded working buffers.
- **64 GB microSD:** notes, incidents, diagnostics, runbooks, logs, snippets, profiles, exports, and configuration data.

That separation is what lets the Terminal and workstation features coexist without sacrificing stability.
