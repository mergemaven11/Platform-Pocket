from pathlib import Path

path = Path("src/main.cpp")
text = path.read_text()

old = '''        if (status.esc)
        {
            if (editorDirty)
                PocketWorkstation::saveEditorFile(editorFileName, editorBuffer);
            editorDirty = false;
            currentScreen = SCREEN_SECTION_MENU;
            drawSectionMenu();
            return;
        }
'''

new = '''        if (status.esc)
        {
            // Never discard a dirty in-memory document when the SD card is
            // missing or a write fails. Stay in the editor with the dirty
            // marker visible so the operator can restore storage and retry.
            if (editorDirty && !PocketWorkstation::saveEditorFile(editorFileName, editorBuffer))
            {
                drawEditor();
                return;
            }
            editorDirty = false;
            currentScreen = SCREEN_SECTION_MENU;
            drawSectionMenu();
            return;
        }
'''

if old not in text:
    raise SystemExit("editor Escape/save anchor not found")

path.write_text(text.replace(old, new, 1))
