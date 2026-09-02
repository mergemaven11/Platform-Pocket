"""Bring the product feature catalog up to date with Platform Pocket v0.7."""

from pathlib import Path

path = Path("docs/FEATURES.md")
source = path.read_text(encoding="utf-8")

source = source.replace(
    "> **Status key:** ✅ Available in v0.6 · 🧭 Planned / placeholder",
    "> **Status key:** ✅ Available in v0.7 · 🧭 Planned / placeholder",
    1,
)

anchor = "### `sysinfo` / `free` — ✅ Available\n"
terminal_storage = '''### `sd` — ✅ Available\n\n**What it does:** Shows the mounted microSD card type, total capacity, used space, free space, and the `/platform-pocket` workspace root.\n\n**Why it matters:** Persistent features are only useful if the operator can confirm that removable storage is actually mounted and healthy.\n\n### `workspace` — ✅ Available\n\n**What it does:** Shows the standard Platform Pocket SD workspace directories for notes, incidents, diagnostics, logs, runbooks, snippets, profiles, exports, and config.\n\n**Why it matters:** A stable filesystem contract keeps future features organized and makes the card easy to inspect from another computer.\n\n### `note TEXT` — ✅ Available\n\n**What it does:** Appends a persistent Markdown quick note to `/platform-pocket/notes/inbox.md`.\n\n**Why it matters:** Troubleshooting observations can survive reboot and remain readable without proprietary software. v0.7 timestamps quick notes with device uptime rather than wall-clock time.\n\n### `notes` — ✅ Available\n\n**What it does:** Reports the persistent notes inbox location and current file size.\n\n**Why it matters:** It gives a fast confirmation that notes exist and are being stored on the SD workspace.\n\n### `snapshot` — ✅ Available\n\n**What it does:** Saves a numbered diagnostic text file under `/platform-pocket/diagnostics/` containing device health and current network state.\n\n**Why it matters:** A durable point-in-time record makes later comparison, incident documentation, and troubleshooting much more reliable than memory or screenshots.\n\n'''
if anchor not in source:
    raise SystemExit("Could not locate terminal feature insertion point")
source = source.replace(anchor, terminal_storage + anchor, 1)

old_notes = '''### View Notes — 🧭 Planned\n\n**What it is:** A planned reader for notes saved on removable storage.\n\n**What it will do:** Display notes stored on the device/SD card.\n\n**Why it matters:** A field device is more useful when it can carry runbooks, IP plans, incident notes, command snippets, and troubleshooting checklists without needing a network connection.\n\n### New Note — 🧭 Planned\n\n**What it is:** A planned keyboard-driven note editor.\n\n**What it will do:** Let the user create notes directly from the Cardputer keyboard.\n\n**Why it matters:** This would allow Platform Pocket to capture information during an incident, site visit, lab session, or troubleshooting task instead of being read-only.\n\n### Delete Note — 🧭 Planned\n\n**What it is:** Planned note-file management.\n\n**What it will do:** Remove saved notes from local/SD storage.\n\n**Why it matters:** Notes need lifecycle management if the device is going to function as a real portable notebook rather than a static reference viewer.\n\n### SD Storage — 🧭 Planned\n\n**What it is:** Planned SD-card status and file information.\n\n**What it will do:** Show capacity, available space, and note/file information stored on the SD card.\n\n**Why it matters:** Once notes and other persistent data use removable storage, the operator needs visibility into whether the card is mounted and how much space remains.\n'''
new_notes = '''### View Notes — ✅ Available\n\n**What it is:** A persistent quick-note inbox backed by the microSD workspace.\n\n**What it does:** Shows the current notes inbox path and size. Quick notes are stored as ordinary Markdown in `/platform-pocket/notes/inbox.md`.\n\n**Why it matters:** Field observations now survive reboot and can be opened later on a computer without conversion or a proprietary format.\n\n### New Note — ✅ Available\n\n**What it is:** The first persistent note-capture workflow.\n\n**What it does:** The Notes menu explains the live `note TEXT` Terminal workflow, which appends the note to the SD-backed Markdown inbox.\n\n**Why it matters:** Platform Pocket can now capture troubleshooting observations instead of functioning only as a read-only utility. A full-screen note editor remains a future UX upgrade.\n\n### Delete Note — 🧭 Planned\n\n**What it is:** Planned note-file lifecycle management with an explicit confirmation flow.\n\n**What it will do:** Safely remove saved notes from the SD workspace.\n\n**Why it matters:** Destructive operations should not be added until the small-screen UI can make deletion deliberate and recoverable enough for field use.\n\n### SD Storage — ✅ Available\n\n**What it is:** Live microSD capacity and workspace health.\n\n**What it does:** Shows card type, total capacity, used space, free space, and the `/platform-pocket` root. The same information is available through the `sd` command.\n\n**Why it matters:** The operator can verify that a large card such as a 64 GB microSD is mounted and available before depending on persistent notes or diagnostics. Platform Pocket never auto-formats a card.\n'''
if old_notes not in source:
    raise SystemExit("Could not locate Notes feature block")
source = source.replace(old_notes, new_notes, 1)

old_storage = '''### Storage — ✅ Available\n\n**What it is:** Internal flash and memory-health information.\n\n**What it does:** Shows flash size, sketch size, remaining sketch space, free heap, and minimum free heap.\n\n**Why it matters:** Embedded systems have tight resource limits. This page makes those limits visible so firmware growth, memory pressure, and available update space can be monitored directly on the device.\n'''
new_storage = '''### Storage — ✅ Available\n\n**What it is:** Combined removable-storage and runtime memory health.\n\n**What it does:** Shows the mounted microSD card capacity/usage and current heap information. The SD workspace is optional, so the page reports an offline state rather than preventing Platform Pocket from booting when a card is absent.\n\n**Why it matters:** v0.7 introduces persistent data, so storage health is now part of operational health alongside embedded memory pressure.\n'''
if old_storage not in source:
    raise SystemExit("Could not locate Settings Storage block")
source = source.replace(old_storage, new_storage, 1)

path.write_text(source, encoding="utf-8")
print("FEATURES.md updated for Platform Pocket v0.7")
