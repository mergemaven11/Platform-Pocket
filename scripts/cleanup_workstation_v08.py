from pathlib import Path

p = Path('src/main.cpp')
s = p.read_text()

s = s.replace('#include "workstation.h"\n#include "workstation.h"', '#include "workstation.h"')

block = '''/** @brief Draw the full-screen Markdown note editor. */
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
while s.count(block) > 1:
    first = s.find(block)
    second = s.find(block, first + len(block))
    s = s[:second] + s[second + len(block):]

# Repeated integration can duplicate editor globals and screen cases too.
s = s.replace('String editorBuffer = "";\nconst size_t EDITOR_MAX_CHARS = 1024;\n\nString editorBuffer = "";\nconst size_t EDITOR_MAX_CHARS = 1024;', 'String editorBuffer = "";\nconst size_t EDITOR_MAX_CHARS = 1024;')
s = s.replace('SCREEN_EDITOR,\n    SCREEN_EDITOR', 'SCREEN_EDITOR')
s = s.replace('PocketWorkstation::begin();\n    PocketWorkstation::begin();', 'PocketWorkstation::begin();')

p.write_text(s)
