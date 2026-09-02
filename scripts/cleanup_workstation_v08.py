from pathlib import Path
import re

p = Path('src/main.cpp')
s = p.read_text()

s = re.sub(r'(?:#include "workstation\.h"\n)+', '#include "workstation.h"\n', s)

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
first = s.find(block)
if first >= 0:
    tail = s[first + len(block):]
    tail = tail.replace(block, '')
    s = s[:first + len(block)] + tail

global_block = 'String editorBuffer = "";\nconst size_t EDITOR_MAX_CHARS = 1024;\n'
first = s.find(global_block)
if first >= 0:
    tail = s[first + len(global_block):]
    tail = tail.replace(global_block, '')
    s = s[:first + len(global_block)] + tail

s = re.sub(r'(?:\s*PocketWorkstation::begin\(\);\n)+', '\n    PocketWorkstation::begin();\n', s)

p.write_text(s)
