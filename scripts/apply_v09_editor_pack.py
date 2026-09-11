from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
MAIN = ROOT / "src" / "main.cpp"
HEADER = ROOT / "src" / "workstation.h"
WORKSTATION = ROOT / "src" / "workstation.cpp"
README = ROOT / "README.md"
DOC = ROOT / "docs" / "PRODUCTION_OPS.md"


def replace_once(text: str, old: str, new: str, label: str) -> str:
    if old not in text:
        raise SystemExit(f"missing anchor: {label}")
    return text.replace(old, new, 1)


main = MAIN.read_text()
main = replace_once(
    main,
    'String editorBuffer = "";\nconst size_t EDITOR_MAX_CHARS = 1024;',
    'String editorBuffer = "";\nString editorFileName = "editor.md";\nbool editorDirty = false;\nconst size_t EDITOR_MAX_CHARS = 8192;',
    "editor globals",
)

old_editor = '''/** @brief Draw the full-screen Markdown note editor. */
void drawEditor()
{
    drawHeader("MARKDOWN EDITOR", String(editorBuffer.length()) + "/" + EDITOR_MAX_CHARS);
    M5Cardputer.Display.fillRect(5, 22, 230, 97, uiPanel);
    M5Cardputer.Display.setTextColor(uiText);
    M5Cardputer.Display.setTextSize(1);
    M5Cardputer.Display.setCursor(8, 26);
    String view = editorBuffer;
    if (view.length() > 420)
        view = view.substring(view.length() - 420);
    M5Cardputer.Display.print(view);
    drawFooter("ENTER save   ESC back   DEL erase");
}

/** @brief Open the persistent Markdown editor. */
void openEditor()
{
    editorBuffer = PocketWorkstation::loadEditorNote();
    if (editorBuffer.length() > EDITOR_MAX_CHARS)
        editorBuffer = editorBuffer.substring(0, EDITOR_MAX_CHARS);
    currentScreen = SCREEN_EDITOR;
    drawEditor();
}
'''
new_editor = '''/** @brief Draw the SD-backed multi-file text editor. */
void drawEditor()
{
    String badge = truncateText(editorFileName, 15);
    if (editorDirty)
        badge += "*";
    drawHeader("TEXT EDITOR", badge);
    M5Cardputer.Display.fillRect(5, 22, 230, 97, uiPanel);
    M5Cardputer.Display.setTextColor(uiText);
    M5Cardputer.Display.setTextSize(1);
    M5Cardputer.Display.setCursor(8, 26);

    // The editor is append-oriented on the tiny 240x135 display. Keep the
    // in-memory document much larger than the visible window and follow the
    // tail while the user types.
    size_t start = editorBuffer.length() > 420 ? editorBuffer.length() - 420 : 0;
    if (start > 0)
    {
        int newline = editorBuffer.indexOf('\\n', start);
        if (newline >= 0 && newline + 1 < static_cast<int>(editorBuffer.length()))
            start = newline + 1;
    }
    String view = editorBuffer.substring(start);
    M5Cardputer.Display.print(view);

    drawFooter(String(editorBuffer.length()) + "/" + EDITOR_MAX_CHARS + "  ENTER save/newline  ESC save");
}

/** @brief Open a named .md/.txt file in the SD-backed editor. */
void openEditor(const String &requestedName = "editor.md")
{
    editorFileName = PocketWorkstation::normalizeEditorFileName(requestedName);
    editorBuffer = PocketWorkstation::loadEditorFile(editorFileName, EDITOR_MAX_CHARS);
    editorBuffer.reserve(EDITOR_MAX_CHARS);
    editorDirty = false;
    currentScreen = SCREEN_EDITOR;
    drawEditor();
}

/** @brief Start a new empty named text file. */
void newEditorFile(const String &requestedName)
{
    editorFileName = PocketWorkstation::normalizeEditorFileName(requestedName);
    editorBuffer = "";
    editorBuffer.reserve(EDITOR_MAX_CHARS);
    editorDirty = true;
    currentScreen = SCREEN_EDITOR;
    drawEditor();
}
'''
main = replace_once(main, old_editor, new_editor, "editor functions")

main = replace_once(
    main,
    '        terminalPush("sd: note notes files snapshot diff");\n        terminalPush("runbooks incident troubleshoot");',
    '        terminalPush("sd: note notes files snapshot diff");\n        terminalPush("text: edit/new/saveas/textfiles");\n        terminalPush("runbooks incident troubleshoot");',
    "help text",
)

old_edit_cmd = '''    else if (lower == "edit")
    {
        openEditor();
        return;
    }
'''
new_edit_cmd = '''    else if (lower == "textfiles")
    {
        terminalPush(PocketWorkstation::editorFileSummary());
    }
    else if (lower == "edit")
    {
        openEditor();
        return;
    }
    else if (lower.startsWith("edit "))
    {
        String name = command.substring(5);
        name.trim();
        if (name.length() == 0)
            terminalPush("usage: edit NAME[.md|.txt]");
        else
        {
            openEditor(name);
            return;
        }
    }
    else if (lower.startsWith("new "))
    {
        String name = command.substring(4);
        name.trim();
        if (name.length() == 0)
            terminalPush("usage: new NAME[.md|.txt]");
        else
        {
            newEditorFile(name);
            return;
        }
    }
    else if (lower.startsWith("saveas "))
    {
        String name = command.substring(7);
        name.trim();
        if (name.length() == 0)
            terminalPush("usage: saveas NAME[.md|.txt]");
        else
        {
            editorFileName = PocketWorkstation::normalizeEditorFileName(name);
            if (PocketWorkstation::saveEditorFile(editorFileName, editorBuffer))
            {
                editorDirty = false;
                terminalPush(String("saved: ") + editorFileName);
            }
            else
                terminalPush("! save failed / SD offline");
        }
    }
'''
main = replace_once(main, old_edit_cmd, new_edit_cmd, "terminal editor commands")

old_loop = '''    if (currentScreen == SCREEN_EDITOR)
    {
        if (status.esc)
        {
            currentScreen = SCREEN_SECTION_MENU;
            drawSectionMenu();
            return;
        }
        if ((status.del || status.backspace) && editorBuffer.length() > 0)
            editorBuffer.remove(editorBuffer.length() - 1);
        for (auto key : status.word)
        {
            if (key >= 32 && key <= 126 && editorBuffer.length() < EDITOR_MAX_CHARS)
                editorBuffer += key;
        }
        if (status.enter)
        {
            if (editorBuffer.length() < EDITOR_MAX_CHARS)
                editorBuffer += '\\n';
            PocketWorkstation::saveEditorNote(editorBuffer);
        }
        drawEditor();
        return;
    }
'''
new_loop = '''    if (currentScreen == SCREEN_EDITOR)
    {
        if (status.esc)
        {
            if (editorDirty)
                PocketWorkstation::saveEditorFile(editorFileName, editorBuffer);
            editorDirty = false;
            currentScreen = SCREEN_SECTION_MENU;
            drawSectionMenu();
            return;
        }
        if ((status.del || status.backspace) && editorBuffer.length() > 0)
        {
            editorBuffer.remove(editorBuffer.length() - 1);
            editorDirty = true;
        }
        for (auto key : status.word)
        {
            if (key >= 32 && key <= 126 && editorBuffer.length() < EDITOR_MAX_CHARS)
            {
                editorBuffer += key;
                editorDirty = true;
            }
        }
        if (status.enter)
        {
            if (editorBuffer.length() < EDITOR_MAX_CHARS)
                editorBuffer += '\\n';
            editorDirty = !PocketWorkstation::saveEditorFile(editorFileName, editorBuffer);
        }
        drawEditor();
        return;
    }
'''
main = replace_once(main, old_loop, new_loop, "editor input loop")
MAIN.write_text(main)

header = HEADER.read_text()
header = replace_once(
    header,
    '''/** @brief Save Markdown text from the full-screen editor. */
bool saveEditorNote(const String &text);

/** @brief Load the current editor note. */
String loadEditorNote();
''',
    '''/** @brief Normalize a user-supplied editor filename to a safe .md/.txt basename. */
String normalizeEditorFileName(const String &name);

/** @brief List editable .md/.txt files in the notes workspace. */
String editorFileSummary();

/** @brief Save text to a named notes workspace file. */
bool saveEditorFile(const String &name, const String &text);

/** @brief Load a bounded named notes workspace file. */
String loadEditorFile(const String &name, size_t limit = 8192);

/** @brief Save Markdown text to the legacy default editor file. */
bool saveEditorNote(const String &text);

/** @brief Load the legacy default editor file. */
String loadEditorNote();
''',
    "workstation header editor api",
)
HEADER.write_text(header)

work = WORKSTATION.read_text()
old_api = '''/** @brief Save the bounded full-screen Markdown editor buffer. */
bool saveEditorNote(const String &text)
{
    return PocketStorage::ready() && writeTextFile(EDITOR_NOTE, text);
}

/** @brief Load the bounded Markdown editor buffer from SD. */
String loadEditorNote()
{
    if (!PocketStorage::ready())
        return "";
    return readTextFile(EDITOR_NOTE, 1024);
}
'''
new_api = '''/** @brief Normalize a requested editor name and preserve .md/.txt intent. */
String normalizeEditorFileName(const String &name)
{
    String value = name;
    value.trim();
    if (value.length() == 0)
        return "editor.md";

    String lower = value;
    lower.toLowerCase();
    String extension = ".md";
    if (lower.endsWith(".txt"))
    {
        extension = ".txt";
        value.remove(value.length() - 4);
    }
    else if (lower.endsWith(".md"))
    {
        value.remove(value.length() - 3);
    }

    String base = slugify(value);
    if (base == "incident" && value.length() == 0)
        base = "editor";
    return base + extension;
}

/** @brief Return a compact list of editable note files. */
String editorFileSummary()
{
    if (!PocketStorage::ready())
        return "SD workspace offline.";

    File dir = SD.open("/platform-pocket/notes");
    if (!dir || !dir.isDirectory())
        return "No notes directory.";

    String out = "TEXT FILES";
    int total = 0;
    int shown = 0;
    File entry = dir.openNextFile();
    while (entry)
    {
        if (!entry.isDirectory())
        {
            String name = entry.name();
            int slash = name.lastIndexOf('/');
            if (slash >= 0)
                name = name.substring(slash + 1);
            String lower = name;
            lower.toLowerCase();
            if (lower.endsWith(".md") || lower.endsWith(".txt"))
            {
                ++total;
                if (shown < 6)
                {
                    out += shown == 0 ? "\\n" : ", ";
                    out += name;
                    ++shown;
                }
            }
        }
        entry.close();
        entry = dir.openNextFile();
    }
    dir.close();

    if (total == 0)
        out += "\\n(none)";
    else if (total > shown)
        out += "\\n+" + String(total - shown) + " more";
    return out;
}

/** @brief Save text to a named .md/.txt file in the notes workspace. */
bool saveEditorFile(const String &name, const String &text)
{
    if (!PocketStorage::ready())
        return false;
    String path = String("/platform-pocket/notes/") + normalizeEditorFileName(name);
    return writeTextFile(path, text);
}

/** @brief Load a bounded named .md/.txt file from the notes workspace. */
String loadEditorFile(const String &name, size_t limit)
{
    if (!PocketStorage::ready())
        return "";
    String path = String("/platform-pocket/notes/") + normalizeEditorFileName(name);
    return readTextFile(path, limit);
}

/** @brief Save to the legacy default editor file for compatibility. */
bool saveEditorNote(const String &text)
{
    return PocketStorage::ready() && writeTextFile(EDITOR_NOTE, text);
}

/** @brief Load the legacy default editor file for compatibility. */
String loadEditorNote()
{
    if (!PocketStorage::ready())
        return "";
    return readTextFile(EDITOR_NOTE, 8192);
}
'''
work = replace_once(work, old_api, new_api, "workstation editor implementation")
WORKSTATION.write_text(work)

readme = README.read_text()
readme = replace_once(
    readme,
    '- Additional offline Kubernetes, container, and incident-response runbooks\n- v0.8 duplicate command/editor integration blocks cleaned up',
    '- Additional offline Kubernetes, container, and incident-response runbooks\n- SD-backed multi-file text editor with an 8 KB working document limit\n- Named `.md` / `.txt` files via `edit`, `new`, `saveas`, and `textfiles`\n- v0.8 duplicate command/editor integration blocks cleaned up',
    "readme highlights",
)
README.write_text(readme)

doc = DOC.read_text()
editor_doc = '''

## Multi-file text editor

Platform Pocket v0.9 expands the original 1 KB Markdown scratchpad into an SD-backed text editor with an 8 KB working-document limit.

Supported terminal workflows:

```text
textfiles
edit
edit maintenance.md
new deploy-notes.txt
saveas incident-copy.md
```

- `edit` opens the default `editor.md` file.
- `edit NAME` opens or creates a named `.md` or `.txt` file under `/platform-pocket/notes/`.
- `new NAME` starts an empty named document.
- `saveas NAME` writes the current editor buffer to another named document.
- `textfiles` lists editable note files.

The editor saves when Enter inserts a newline and again when leaving with Escape. The 8 KB cap is intentionally conservative for ESP32-S3 heap stability; the SD card itself can store much larger collections of text files.
'''
if "## Multi-file text editor" not in doc:
    doc += editor_doc
DOC.write_text(doc)
