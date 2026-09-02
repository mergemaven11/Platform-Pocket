from pathlib import Path

p = Path('src/main.cpp')
s = p.read_text()

s = s.replace('#include "storage.h"', '#include "storage.h"\n#include "workstation.h"')
s = s.replace('// PLATFORM POCKET v0.7', '// PLATFORM POCKET v0.8')
s = s.replace('static const char *APP_VERSION = "0.7";', 'static const char *APP_VERSION = "0.8";')
s = s.replace('    SCREEN_THEME\n};', '    SCREEN_THEME,\n    SCREEN_EDITOR\n};')

anchor = 'String previousTerminalCommand = "";\n'
insert = '''String previousTerminalCommand = "";\n\nString editorBuffer = "";\nconst size_t EDITOR_MAX_CHARS = 1024;\n'''
s = s.replace(anchor, insert)

anchor = '/**\n * @brief Document runTerminalCommand.\n */\nvoid runTerminalCommand()'
editor_code = r'''/** @brief Draw the full-screen Markdown note editor. */
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
s = s.replace(anchor, editor_code + anchor)

old_help = '''        terminalPush("sd workspace note TEXT");\n        terminalPush("notes snapshot uptime version");'''
new_help = '''        terminalPush("sd workspace note TEXT notes");\n        terminalPush("files edit snapshot diff runbooks");\n        terminalPush("runbook NAME incident ...");\n        terminalPush("troubleshoot TEXT uptime version");'''
s = s.replace(old_help, new_help)

anchor = '''    else if (lower == "snapshot")
    {
        String result = PocketStorage::saveDiagnosticSnapshot();
        terminalPush(result.startsWith("!") ? result : String("saved: ") + result);
    }
'''
extra = r'''    else if (lower == "files")
    {
        terminalPush(PocketWorkstation::fileManagerSummary());
    }
    else if (lower == "edit")
    {
        openEditor();
        return;
    }
    else if (lower == "diff")
    {
        terminalPush(PocketWorkstation::compareSnapshots());
    }
    else if (lower == "runbooks")
    {
        terminalPush(PocketWorkstation::runbookSummary());
    }
    else if (lower.startsWith("runbook "))
    {
        terminalPush(PocketWorkstation::readRunbook(command.substring(8)));
    }
    else if (lower.startsWith("incident new "))
    {
        terminalPush(PocketWorkstation::createIncident(command.substring(13)));
    }
    else if (lower.startsWith("incident add "))
    {
        terminalPush(PocketWorkstation::appendIncident(command.substring(13)) ? "incident updated" : "! incident append failed");
    }
    else if (lower == "incident")
    {
        terminalPush(PocketWorkstation::incidentSummary());
    }
    else if (lower.startsWith("troubleshoot "))
    {
        terminalPush(PocketWorkstation::troubleshoot(command.substring(13)));
    }
'''
s = s.replace(anchor, anchor + extra)

s = s.replace('''        case 0:
            openTool("VIEW NOTES", PocketStorage::noteSummary());
            break;
        case 1:
            openTool("NEW NOTE", "Persistent quick notes are live.\\n\\nTerminal:\\nnote your text\\n\\nSaved to notes/inbox.md");
            break;
        case 2:
            openTool("DELETE NOTE", "Saved note management is\\nqueued with SD support.");
            break;
        case 3:
            openTool("SD STORAGE", PocketStorage::statusText());
            break;''', '''        case 0:
            openTool("FILE MANAGER", PocketWorkstation::fileManagerSummary());
            break;
        case 1:
            openEditor();
            break;
        case 2:
            openTool("INCIDENT NOTEBOOK", PocketWorkstation::incidentSummary());
            break;
        case 3:
            openTool("SD STORAGE", PocketStorage::statusText());
            break;''')

s = s.replace('const char *notesMenu[] = {\n    "View Notes",\n    "New Note",\n    "Delete Note",\n    "SD Storage"};', 'const char *notesMenu[] = {\n    "File Manager",\n    "Markdown Editor",\n    "Incident Notebook",\n    "SD Storage"};')

s = s.replace('''    PocketStorage::begin();
    applyTheme();''', '''    PocketStorage::begin();
    PocketWorkstation::begin();
    applyTheme();''')

loop_anchor = '''    if (currentScreen == SCREEN_TERMINAL)
    {'''
editor_loop = r'''    if (currentScreen == SCREEN_EDITOR)
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
                editorBuffer += '\n';
            PocketWorkstation::saveEditorNote(editorBuffer);
        }
        drawEditor();
        return;
    }

'''
s = s.replace(loop_anchor, editor_loop + loop_anchor)

s = s.replace('text += "\\n\\nSD workspace + persistence.";', 'text += "\\n\\nPocket workstation + SD.";')

p.write_text(s)
