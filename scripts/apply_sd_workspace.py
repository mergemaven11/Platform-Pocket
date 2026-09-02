"""Integrate the SD workspace module into Platform Pocket's main firmware."""

from pathlib import Path

path = Path("src/main.cpp")
source = path.read_text(encoding="utf-8")


def replace_once(old: str, new: str) -> None:
    """Replace one required source fragment or fail loudly."""
    global source
    if old not in source:
        raise SystemExit(f"Expected fragment not found:\n{old[:160]}")
    source = source.replace(old, new, 1)


replace_once(
    '#include <mbedtls/sha256.h>\n',
    '#include <mbedtls/sha256.h>\n#include "storage.h"\n',
)
replace_once('// PLATFORM POCKET v0.6', '// PLATFORM POCKET v0.7')
replace_once('static const char *APP_VERSION = "0.6";', 'static const char *APP_VERSION = "0.7";')
replace_once(
    '        terminalPush("wifi scan ip net dns HOST");\n        terminalPush("port HOST PORT sha256 TEXT");\n        terminalPush("cidr N base N diag sysinfo");\n        terminalPush("uptime docker history version");',
    '        terminalPush("wifi scan ip net dns HOST");\n        terminalPush("port HOST PORT sha256 TEXT");\n        terminalPush("cidr N base N diag sysinfo");\n        terminalPush("sd workspace note TEXT");\n        terminalPush("notes snapshot uptime version");',
)
anchor = '''    else if (lower == "sysinfo" || lower == "free")\n    {\n'''
insert = r'''    else if (lower == "sd")
    {
        terminalPush(PocketStorage::statusText());
    }
    else if (lower == "workspace")
    {
        terminalPush(PocketStorage::workspaceSummary());
    }
    else if (lower.startsWith("note "))
    {
        String text = command.substring(5);
        text.trim();
        if (text.length() == 0)
            terminalPush("usage: note TEXT");
        else if (PocketStorage::appendQuickNote(text))
            terminalPush("note saved: notes/inbox.md");
        else
            terminalPush("! note save failed / SD offline");
    }
    else if (lower == "notes")
    {
        terminalPush(PocketStorage::noteSummary());
    }
    else if (lower == "snapshot")
    {
        String result = PocketStorage::saveDiagnosticSnapshot();
        terminalPush(result.startsWith("!") ? result : String("saved: ") + result);
    }
'''
replace_once(anchor, insert + anchor)

replace_once(
    '            openTool("VIEW NOTES", "SD-backed notes are queued.\\n\\nThe redesigned shell now\\nprovides keyboard input.");',
    '            openTool("VIEW NOTES", PocketStorage::noteSummary());',
)
replace_once(
    '            openTool("NEW NOTE", "Note editor is queued for\\nthe next storage pass.");',
    '            openTool("NEW NOTE", "Persistent quick notes are live.\\n\\nTerminal:\\nnote your text\\n\\nSaved to notes/inbox.md");',
)
replace_once(
    '            openTool("SD STORAGE", "SD capacity, free space\\nand note files are queued.");',
    '            openTool("SD STORAGE", PocketStorage::statusText());',
)
replace_once(
    '            showStorageInfo();',
    '            openTool("STORAGE", PocketStorage::statusText() + "\\n\\n" + String("Heap: ") + ESP.getFreeHeap());',
    )
replace_once(
    '    M5Cardputer.Display.setBrightness(screenBrightness);\n    applyTheme();',
    '    M5Cardputer.Display.setBrightness(screenBrightness);\n    PocketStorage::begin();\n    applyTheme();',
)
replace_once(
    '    text += "\\n\\nField tools + diagnostics.";',
    '    text += "\\n\\nSD workspace + persistence.";',
)

path.write_text(source, encoding="utf-8")
print("Platform Pocket v0.7 SD workspace integration applied.")
