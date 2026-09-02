#include <M5Cardputer.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <esp_system.h>
#include <mbedtls/sha256.h>
#include "storage.h"
#include "workstation.h"
#include "workstation.h"
#include "workstation.h"

// ============================================================
// PLATFORM POCKET v0.8
// A compact handheld platform / network toolkit for Cardputer ADV.
// ============================================================

static const char *APP_VERSION = "0.8";

// 240x135 Cardputer display palette. Values are RGB565.
uint16_t uiBg = 0x0841;
uint16_t uiHeader = 0x10A2;
uint16_t uiPanel = 0x18E3;
uint16_t uiPanelAlt = 0x2124;
uint16_t uiAccent = 0x4DFF;
uint16_t uiText = 0xFFFF;
uint16_t uiMuted = 0x9CD3;
uint16_t uiSuccess = 0x47E8;
uint16_t uiWarning = 0xFD20;

int themeIndex = 0;
int screenBrightness = 160;

enum MainMenuItem
{
    MAIN_NETWORK,
    MAIN_SECURITY,
    MAIN_DOCKER,
    MAIN_TERMINAL,
    MAIN_NOTES,
    MAIN_TOOLS,
    MAIN_SETTINGS,
    MAIN_COUNT
};

enum ScreenState
{
    SCREEN_MAIN,
    SCREEN_SECTION_MENU,
    SCREEN_TOOL,
    SCREEN_WIFI_SCAN,
    SCREEN_WIFI_DETAILS,
    SCREEN_TERMINAL,
    SCREEN_BRIGHTNESS,
    SCREEN_THEME,
    SCREEN_EDITOR
};

ScreenState currentScreen = SCREEN_MAIN;
int selectedMainItem = 0;
int selectedSubItem[MAIN_COUNT] = {0, 0, 0, 0, 0, 0, 0};

const char *mainMenuItems[] = {
    "Network",
    "Security",
    "Docker",
    "Terminal",
    "Notes",
    "Tools",
    "Settings"};

const char *mainMenuTags[] = {
    "NET", "SAFE", "CTR", ">_", "TXT", "UTIL", "CFG"};

const char *networkMenu[] = {
    "Wi-Fi Scanner",
    "Wi-Fi Info",
    "Signal Monitor",
    "DNS Lookup",
    "Network Tools"};
const int networkMenuCount = 5;

const char *securityMenu[] = {
    "Security Dashboard",
    "Wi-Fi Observations",
    "Device Info",
    "Hash Tool",
    "Port Check"};
const int securityMenuCount = 5;

const char *dockerMenu[] = {
    "Docker Commands",
    "Container Cheatsheet",
    "Compose Cheatsheet",
    "Remote Host"};
const int dockerMenuCount = 4;

const char *notesMenu[] = {
    "File Manager",
    "Markdown Editor",
    "Incident Notebook",
    "SD Storage"};
const int notesMenuCount = 4;

const char *toolsMenu[] = {
    "IP Tools",
    "Subnet Helper",
    "Base Converter",
    "Password Generator",
    "System Info"};
const int toolsMenuCount = 5;

const char *settingsMenu[] = {
    "Brightness",
    "Theme",
    "Wi-Fi Settings",
    "Storage",
    "About"};
const int settingsMenuCount = 5;

int wifiNetworkCount = 0;
int selectedWifiNetwork = 0;
int wifiScrollOffset = 0;
const int wifiRowsVisible = 4;

String toolTitle = "";
String toolMessage = "";

String terminalInput = "";
String terminalLines[6];
int terminalLineCount = 0;
String lastTerminalCommand = "";
String previousTerminalCommand = "";

String editorBuffer = "";
const size_t EDITOR_MAX_CHARS = 1024;

String editorBuffer = "";
const size_t EDITOR_MAX_CHARS = 1024;

String editorBuffer = "";
const size_t EDITOR_MAX_CHARS = 1024;

// Forward declarations for terminal field-tool helpers.
String formatIpv4(uint32_t value);
String toBinary(uint32_t value);
String sha256Text(const String &value);
void showSignalMonitor();
void showNetworkTools();
void showHashToolHelp();
void showPortCheckHelp();
void showIpTools();
void showSubnetHelper();
void showBaseConverter();
void showStorageInfo();

/**
 * @brief Document applyTheme.
 */
void applyTheme()
{
    if (themeIndex == 0)
    {
        uiBg = 0x0841;
        uiHeader = 0x10A2;
        uiPanel = 0x18E3;
        uiPanelAlt = 0x2124;
        uiAccent = 0x4DFF;
        uiMuted = 0x9CD3;
    }
    else if (themeIndex == 1)
    {
        uiBg = 0x0020;
        uiHeader = 0x0840;
        uiPanel = 0x0861;
        uiPanelAlt = 0x10A2;
        uiAccent = 0x07E0;
        uiMuted = 0x7BEF;
    }
    else
    {
        uiBg = 0x1000;
        uiHeader = 0x2000;
        uiPanel = 0x3000;
        uiPanelAlt = 0x4000;
        uiAccent = 0xFD20;
        uiMuted = 0xC618;
    }
}

/**
 * @brief Document getThemeName.
 */
const char *getThemeName()
{
    if (themeIndex == 0)
        return "MIDNIGHT CYAN";
    if (themeIndex == 1)
        return "MATRIX GREEN";
    return "AMBER OPS";
}

/**
 * @brief Document drawHeader.
 */
void drawHeader(const String &title, const String &badge = "")
{
    M5Cardputer.Display.fillScreen(uiBg);
    M5Cardputer.Display.fillRect(0, 0, 240, 19, uiHeader);
    M5Cardputer.Display.drawFastHLine(0, 19, 240, uiPanelAlt);

    M5Cardputer.Display.setTextSize(1);
    M5Cardputer.Display.setTextColor(uiText);
    M5Cardputer.Display.setCursor(6, 6);
    M5Cardputer.Display.print(title);

    if (badge.length() > 0)
    {
        M5Cardputer.Display.setTextColor(uiMuted);
        int badgeX = 236 - (badge.length() * 6);
        if (badgeX < 150)
            badgeX = 150;
        M5Cardputer.Display.setCursor(badgeX, 6);
        M5Cardputer.Display.print(badge);
    }
}

/**
 * @brief Document drawStatusHeader.
 */
void drawStatusHeader()
{
    drawHeader("PLATFORM POCKET", String("v") + APP_VERSION);

    uint16_t dotColor = WiFi.status() == WL_CONNECTED ? uiSuccess : uiMuted;
    M5Cardputer.Display.fillRect(174, 7, 5, 5, dotColor);
    M5Cardputer.Display.setTextColor(uiMuted);
    M5Cardputer.Display.setCursor(182, 6);
    M5Cardputer.Display.print(WiFi.status() == WL_CONNECTED ? "NET" : "OFF");
}

/**
 * @brief Document drawFooter.
 */
void drawFooter(const String &hint)
{
    M5Cardputer.Display.fillRect(0, 121, 240, 14, uiHeader);
    M5Cardputer.Display.drawFastHLine(0, 120, 240, uiPanelAlt);
    M5Cardputer.Display.setTextSize(1);
    M5Cardputer.Display.setTextColor(uiMuted);
    M5Cardputer.Display.setCursor(6, 125);
    M5Cardputer.Display.print(hint);
}

/**
 * @brief Document truncateText.
 */
String truncateText(const String &value, int maxChars)
{
    if ((int)value.length() <= maxChars)
        return value;
    if (maxChars <= 3)
        return value.substring(0, maxChars);
    return value.substring(0, maxChars - 3) + "...";
}

/**
 * @brief Document getSectionTitle.
 */
const char *getSectionTitle()
{
    return mainMenuItems[selectedMainItem];
}

/**
 * @brief Document getCurrentSubMenu.
 */
const char **getCurrentSubMenu()
{
    switch (selectedMainItem)
    {
    case MAIN_NETWORK:
        return networkMenu;
    case MAIN_SECURITY:
        return securityMenu;
    case MAIN_DOCKER:
        return dockerMenu;
    case MAIN_NOTES:
        return notesMenu;
    case MAIN_TOOLS:
        return toolsMenu;
    case MAIN_SETTINGS:
        return settingsMenu;
    default:
        return networkMenu;
    }
}

/**
 * @brief Document getCurrentSubMenuCount.
 */
int getCurrentSubMenuCount()
{
    switch (selectedMainItem)
    {
    case MAIN_NETWORK:
        return networkMenuCount;
    case MAIN_SECURITY:
        return securityMenuCount;
    case MAIN_DOCKER:
        return dockerMenuCount;
    case MAIN_NOTES:
        return notesMenuCount;
    case MAIN_TOOLS:
        return toolsMenuCount;
    case MAIN_SETTINGS:
        return settingsMenuCount;
    default:
        return 0;
    }
}

/**
 * @brief Document getSignalLabel.
 */
const char *getSignalLabel(int rssi)
{
    if (rssi >= -50)
        return "STRONG";
    if (rssi >= -65)
        return "GOOD";
    if (rssi >= -75)
        return "FAIR";
    return "WEAK";
}

/**
 * @brief Document wifiIsOpen.
 */
bool wifiIsOpen(int index)
{
    return WiFi.encryptionType(index) == WIFI_AUTH_OPEN;
}

/**
 * @brief Document hasDuplicateSSID.
 */
bool hasDuplicateSSID(int index)
{
    String target = WiFi.SSID(index);
    if (target.length() == 0)
        return false;

    for (int i = 0; i < wifiNetworkCount; i++)
    {
        if (i != index && WiFi.SSID(i) == target)
            return true;
    }
    return false;
}

/**
 * @brief Document drawMainMenu.
 */
void drawMainMenu()
{
    drawStatusHeader();

    const int rowHeight = 19;
    const int visibleRows = 5;
    int first = selectedMainItem - 2;
    if (first < 0)
        first = 0;
    if (first > MAIN_COUNT - visibleRows)
        first = MAIN_COUNT - visibleRows;
    if (first < 0)
        first = 0;

    for (int row = 0; row < visibleRows; row++)
    {
        int item = first + row;
        if (item >= MAIN_COUNT)
            break;

        int y = 24 + row * rowHeight;
        bool selected = item == selectedMainItem;

        if (selected)
        {
            M5Cardputer.Display.fillRect(5, y, 230, 17, uiAccent);
            M5Cardputer.Display.setTextColor(uiBg);
        }
        else
        {
            M5Cardputer.Display.fillRect(5, y, 230, 17, (row % 2 == 0) ? uiPanel : uiPanelAlt);
            M5Cardputer.Display.setTextColor(uiText);
        }

        M5Cardputer.Display.setCursor(11, y + 5);
        M5Cardputer.Display.print(selected ? "> " : "  ");
        M5Cardputer.Display.print(mainMenuItems[item]);

        M5Cardputer.Display.setCursor(202, y + 5);
        M5Cardputer.Display.print(mainMenuTags[item]);
    }

    drawFooter(";/. move   ENTER open");
}

/**
 * @brief Document drawSectionMenu.
 */
void drawSectionMenu()
{
    drawHeader(String("< ") + getSectionTitle(), "TOOLS");

    const char **menu = getCurrentSubMenu();
    int count = getCurrentSubMenuCount();
    int selected = selectedSubItem[selectedMainItem];

    const int rowHeight = 19;
    for (int i = 0; i < count; i++)
    {
        int y = 24 + i * rowHeight;
        bool active = i == selected;

        M5Cardputer.Display.fillRect(5, y, 230, 17, active ? uiAccent : ((i % 2 == 0) ? uiPanel : uiPanelAlt));
        M5Cardputer.Display.setTextColor(active ? uiBg : uiText);
        M5Cardputer.Display.setCursor(11, y + 5);
        M5Cardputer.Display.print(active ? "> " : "  ");
        M5Cardputer.Display.print(menu[i]);
    }

    drawFooter(";/. move   ENTER open   ESC back");
}

/**
 * @brief Document drawToolPage.
 */
void drawToolPage()
{
    drawHeader(toolTitle, "INFO");
    M5Cardputer.Display.fillRect(5, 24, 230, 91, uiPanel);
    M5Cardputer.Display.setTextColor(uiText);
    M5Cardputer.Display.setTextSize(1);
    M5Cardputer.Display.setCursor(10, 29);
    M5Cardputer.Display.println(toolMessage);
    drawFooter("ESC back");
}

/**
 * @brief Document openTool.
 */
void openTool(const String &title, const String &message)
{
    toolTitle = title;
    toolMessage = message;
    currentScreen = SCREEN_TOOL;
    drawToolPage();
}

/**
 * @brief Document drawScanningScreen.
 */
void drawScanningScreen()
{
    drawHeader("WI-FI SCANNER", "SCAN");
    M5Cardputer.Display.fillRect(12, 38, 216, 55, uiPanel);
    M5Cardputer.Display.setTextColor(uiAccent);
    M5Cardputer.Display.setTextSize(2);
    M5Cardputer.Display.setCursor(39, 48);
    M5Cardputer.Display.print("SCANNING");
    M5Cardputer.Display.setTextSize(1);
    M5Cardputer.Display.setTextColor(uiMuted);
    M5Cardputer.Display.setCursor(45, 77);
    M5Cardputer.Display.print("nearby access points");
    drawFooter("radio busy...");
}

/**
 * @brief Document scanWifiNetworks.
 */
void scanWifiNetworks()
{
    drawScanningScreen();
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(150);
    WiFi.scanDelete();
    wifiNetworkCount = WiFi.scanNetworks(false, true);
    if (wifiNetworkCount < 0)
        wifiNetworkCount = 0;
    selectedWifiNetwork = 0;
    wifiScrollOffset = 0;
    currentScreen = SCREEN_WIFI_SCAN;
}

/**
 * @brief Document drawWifiResults.
 */
void drawWifiResults()
{
    drawHeader("WI-FI SCANNER", String(wifiNetworkCount) + " FOUND");

    if (wifiNetworkCount <= 0)
    {
        M5Cardputer.Display.fillRect(10, 38, 220, 55, uiPanel);
        M5Cardputer.Display.setTextColor(uiText);
        M5Cardputer.Display.setCursor(45, 52);
        M5Cardputer.Display.print("No networks found");
        M5Cardputer.Display.setTextColor(uiMuted);
        M5Cardputer.Display.setCursor(37, 70);
        M5Cardputer.Display.print("ENTER to scan again");
        drawFooter("ENTER rescan   ESC back");
        return;
    }

    int finalRow = wifiScrollOffset + wifiRowsVisible;
    if (finalRow > wifiNetworkCount)
        finalRow = wifiNetworkCount;

    for (int i = wifiScrollOffset; i < finalRow; i++)
    {
        int row = i - wifiScrollOffset;
        int y = 23 + row * 24;
        bool active = i == selectedWifiNetwork;
        uint16_t bg = active ? uiAccent : ((row % 2 == 0) ? uiPanel : uiPanelAlt);
        M5Cardputer.Display.fillRect(4, y, 232, 22, bg);

        String ssid = WiFi.SSID(i);
        if (ssid.length() == 0)
            ssid = "<hidden>";
        ssid = truncateText(ssid, 22);

        M5Cardputer.Display.setTextColor(active ? uiBg : uiText);
        M5Cardputer.Display.setCursor(8, y + 3);
        M5Cardputer.Display.print(active ? "> " : "  ");
        M5Cardputer.Display.print(ssid);

        M5Cardputer.Display.setTextColor(active ? uiBg : uiMuted);
        M5Cardputer.Display.setCursor(20, y + 13);
        M5Cardputer.Display.print(WiFi.RSSI(i));
        M5Cardputer.Display.print(" dBm  CH");
        M5Cardputer.Display.print(WiFi.channel(i));
        M5Cardputer.Display.print(wifiIsOpen(i) ? "  OPEN" : "  LOCK");
    }

    drawFooter(";/. move  ENTER detail  ESC back");
}

/**
 * @brief Document drawWifiDetails.
 */
void drawWifiDetails()
{
    if (wifiNetworkCount <= 0)
        return;

    drawHeader("NETWORK DETAIL", String(selectedWifiNetwork + 1) + "/" + String(wifiNetworkCount));
    M5Cardputer.Display.fillRect(5, 24, 230, 91, uiPanel);

    String ssid = WiFi.SSID(selectedWifiNetwork);
    if (ssid.length() == 0)
        ssid = "<hidden>";
    int rssi = WiFi.RSSI(selectedWifiNetwork);

    M5Cardputer.Display.setTextSize(1);
    M5Cardputer.Display.setTextColor(uiMuted);
    M5Cardputer.Display.setCursor(10, 30);
    M5Cardputer.Display.print("SSID");
    M5Cardputer.Display.setTextColor(uiText);
    M5Cardputer.Display.setCursor(58, 30);
    M5Cardputer.Display.print(truncateText(ssid, 27));

    M5Cardputer.Display.setTextColor(uiMuted);
    M5Cardputer.Display.setCursor(10, 45);
    M5Cardputer.Display.print("BSSID");
    M5Cardputer.Display.setTextColor(uiText);
    M5Cardputer.Display.setCursor(58, 45);
    M5Cardputer.Display.print(WiFi.BSSIDstr(selectedWifiNetwork));

    M5Cardputer.Display.setTextColor(uiMuted);
    M5Cardputer.Display.setCursor(10, 60);
    M5Cardputer.Display.print("SIGNAL");
    M5Cardputer.Display.setTextColor(uiText);
    M5Cardputer.Display.setCursor(58, 60);
    M5Cardputer.Display.print(rssi);
    M5Cardputer.Display.print(" dBm  ");
    M5Cardputer.Display.print(getSignalLabel(rssi));

    M5Cardputer.Display.setTextColor(uiMuted);
    M5Cardputer.Display.setCursor(10, 75);
    M5Cardputer.Display.print("CHANNEL");
    M5Cardputer.Display.setTextColor(uiText);
    M5Cardputer.Display.setCursor(58, 75);
    M5Cardputer.Display.print(WiFi.channel(selectedWifiNetwork));

    M5Cardputer.Display.setTextColor(uiMuted);
    M5Cardputer.Display.setCursor(10, 90);
    M5Cardputer.Display.print("SECURITY");
    M5Cardputer.Display.setTextColor(wifiIsOpen(selectedWifiNetwork) ? uiWarning : uiSuccess);
    M5Cardputer.Display.setCursor(58, 90);
    M5Cardputer.Display.print(wifiIsOpen(selectedWifiNetwork) ? "OPEN" : "SECURED");

    M5Cardputer.Display.setTextColor(uiMuted);
    M5Cardputer.Display.setCursor(10, 105);
    M5Cardputer.Display.print("DUP SSID");
    M5Cardputer.Display.setTextColor(uiText);
    M5Cardputer.Display.setCursor(58, 105);
    M5Cardputer.Display.print(hasDuplicateSSID(selectedWifiNetwork) ? "YES" : "NO");

    drawFooter("ESC back");
}

/**
 * @brief Document terminalClear.
 */
void terminalClear()
{
    terminalLineCount = 0;
    terminalInput = "";
    for (int i = 0; i < 6; i++)
        terminalLines[i] = "";
}

/**
 * @brief Document terminalPush.
 */
void terminalPush(const String &line)
{
    if (terminalLineCount < 6)
    {
        terminalLines[terminalLineCount++] = line;
        return;
    }

    for (int i = 0; i < 5; i++)
        terminalLines[i] = terminalLines[i + 1];
    terminalLines[5] = line;
}

/**
 * @brief Document drawTerminal.
 */
void drawTerminal()
{
    drawHeader("TERMINAL", "LOCAL");
    M5Cardputer.Display.fillRect(4, 23, 232, 78, uiPanel);

    M5Cardputer.Display.setTextSize(1);
    for (int i = 0; i < terminalLineCount; i++)
    {
        int y = 27 + i * 12;
        M5Cardputer.Display.setTextColor(terminalLines[i].startsWith("!") ? uiWarning : uiText);
        M5Cardputer.Display.setCursor(8, y);
        M5Cardputer.Display.print(truncateText(terminalLines[i], 37));
    }

    M5Cardputer.Display.fillRect(4, 104, 232, 14, uiPanelAlt);
    M5Cardputer.Display.setTextColor(uiAccent);
    M5Cardputer.Display.setCursor(8, 108);
    M5Cardputer.Display.print("pocket> ");
    M5Cardputer.Display.setTextColor(uiText);
    M5Cardputer.Display.print(truncateText(terminalInput, 27));
    M5Cardputer.Display.fillRect(224, 108, 5, 7, uiAccent);

    drawFooter("ENTER run   DEL edit   ESC back");
}

/** @brief Draw the full-screen Markdown note editor. */
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

/** @brief Draw the full-screen Markdown note editor. */
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

/** @brief Draw the full-screen Markdown note editor. */
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

/**
 * @brief Document runTerminalCommand.
 */
void runTerminalCommand()
{
    String command = terminalInput;
    command.trim();
    terminalInput = "";

    if (command.length() == 0)
    {
        drawTerminal();
        return;
    }

    previousTerminalCommand = lastTerminalCommand;
    lastTerminalCommand = command;
    terminalPush(String("> ") + command);

    String lower = command;
    lower.toLowerCase();

    if (lower == "help" || lower == "?")
    {
        terminalPush("wifi scan ip net dns HOST");
        terminalPush("port HOST PORT sha256 TEXT");
        terminalPush("cidr N base N diag sysinfo");
        terminalPush("sd workspace note TEXT notes");
        terminalPush("files edit snapshot diff runbooks");
        terminalPush("runbook NAME incident ...");
        terminalPush("troubleshoot TEXT uptime version");
    }
    else if (lower == "clear" || lower == "cls")
    {
        terminalClear();
    }
    else if (lower == "wifi")
    {
        if (WiFi.status() == WL_CONNECTED)
        {
            terminalPush(String("ssid: ") + truncateText(WiFi.SSID(), 28));
            terminalPush(String("rssi: ") + WiFi.RSSI() + " dBm");
        }
        else
        {
            terminalPush("wifi: not connected");
        }
    }
    else if (lower == "scan")
    {
        terminalPush("scanning...");
        drawTerminal();
        WiFi.mode(WIFI_STA);
        int found = WiFi.scanNetworks(false, true);
        terminalPush(String("found: ") + found + " networks");
        int showCount = found < 2 ? found : 2;
        for (int i = 0; i < showCount; i++)
        {
            String ssid = WiFi.SSID(i);
            if (ssid.length() == 0)
                ssid = "<hidden>";
            terminalPush(truncateText(ssid, 22) + " " + String(WiFi.RSSI(i)) + "dBm");
        }
        WiFi.scanDelete();
    }
    else if (lower == "ip")
    {
        if (WiFi.status() == WL_CONNECTED)
        {
            terminalPush(String("ip: ") + WiFi.localIP().toString());
            terminalPush(String("gw: ") + WiFi.gatewayIP().toString());
        }
        else
        {
            terminalPush("! connect to Wi-Fi first");
        }
    }
    else if (lower == "net")
    {
        if (WiFi.status() != WL_CONNECTED)
        {
            terminalPush("! connect to Wi-Fi first");
        }
        else
        {
            terminalPush(String("ip: ") + WiFi.localIP().toString());
            terminalPush(String("gw: ") + WiFi.gatewayIP().toString());
            terminalPush(String("mask: ") + WiFi.subnetMask().toString());
            terminalPush(String("dns: ") + WiFi.dnsIP().toString());
        }
    }
    else if (lower.startsWith("dns "))
    {
        if (WiFi.status() != WL_CONNECTED)
        {
            terminalPush("! connect to Wi-Fi first");
        }
        else
        {
            String host = command.substring(4);
            host.trim();
            IPAddress resolved;
            int ok = host.length() > 0 ? WiFi.hostByName(host.c_str(), resolved) : 0;
            if (ok == 1)
                terminalPush(host + " -> " + resolved.toString());
            else
                terminalPush("! DNS lookup failed");
        }
    }
    else if (lower.startsWith("port "))
    {
        if (WiFi.status() != WL_CONNECTED)
        {
            terminalPush("! connect to Wi-Fi first");
        }
        else
        {
            int split = command.indexOf(' ', 5);
            if (split < 0)
            {
                terminalPush("usage: port HOST PORT");
            }
            else
            {
                String host = command.substring(5, split);
                int port = command.substring(split + 1).toInt();
                if (port < 1 || port > 65535)
                {
                    terminalPush("! invalid TCP port");
                }
                else
                {
                    WiFiClient client;
                    bool reachable = client.connect(host.c_str(), static_cast<uint16_t>(port), 1200);
                    terminalPush(host + ":" + String(port) + (reachable ? " OPEN" : " CLOSED/NO REPLY"));
                    client.stop();
                }
            }
        }
    }
    else if (lower.startsWith("sha256 "))
    {
        String value = command.substring(7);
        if (value.length() == 0)
            terminalPush("usage: sha256 TEXT");
        else
        {
            String digest = sha256Text(value);
            terminalPush(digest.substring(0, 32));
            terminalPush(digest.substring(32));
        }
    }
    else if (lower.startsWith("cidr "))
    {
        int prefix = command.substring(5).toInt();
        if (prefix < 0 || prefix > 32)
        {
            terminalPush("usage: cidr 0..32");
        }
        else
        {
            uint32_t mask = prefix == 0 ? 0U : (0xFFFFFFFFUL << (32 - prefix));
            uint64_t addresses = prefix == 32 ? 1ULL : (1ULL << (32 - prefix));
            terminalPush(String("/") + prefix + " mask " + formatIpv4(mask));
            terminalPush(String("addresses: ") + static_cast<unsigned long>(addresses > 0xFFFFFFFFULL ? 0xFFFFFFFFUL : addresses));
        }
    }
    else if (lower.startsWith("base "))
    {
        String raw = command.substring(5);
        raw.trim();
        char *end = nullptr;
        unsigned long value = strtoul(raw.c_str(), &end, 10);
        if (raw.length() == 0 || end == raw.c_str() || *end != '\0')
        {
            terminalPush("usage: base DECIMAL");
        }
        else
        {
            terminalPush(String("dec: ") + value);
            terminalPush(String("hex: 0x") + String(value, HEX));
            terminalPush(String("bin: ") + truncateText(toBinary(value), 30));
        }
    }
    else if (lower == "diag")
    {
        terminalPush(String("wifi: ") + (WiFi.status() == WL_CONNECTED ? "connected" : "offline"));
        terminalPush(String("heap: ") + ESP.getFreeHeap());
        terminalPush(String("minheap: ") + ESP.getMinFreeHeap());
        terminalPush(String("uptime: ") + millis() / 1000UL + " sec");
    }
    else if (lower == "sd")
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
    else if (lower == "files")
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
    else if (lower == "files")
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
    else if (lower == "files")
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
    else if (lower == "sysinfo" || lower == "free")
    {
        terminalPush(String("cpu: ") + getCpuFrequencyMhz() + " MHz");
        terminalPush(String("heap: ") + ESP.getFreeHeap() + " bytes");
        terminalPush(String("flash: ") + ESP.getFlashChipSize() + " bytes");
    }
    else if (lower == "uptime")
    {
        unsigned long seconds = millis() / 1000UL;
        terminalPush(String("uptime: ") + seconds + " sec");
    }
    else if (lower == "docker")
    {
        terminalPush("docker ps | images | logs");
        terminalPush("compose up -d | ps | down");
    }
    else if (lower == "history")
    {
        terminalPush(previousTerminalCommand.length() ? String("last: ") + previousTerminalCommand : "history empty");
    }
    else if (lower == "version")
    {
        terminalPush(String("Platform Pocket v") + APP_VERSION);
    }
    else if (lower.startsWith("echo "))
    {
        terminalPush(command.substring(5));
    }
    else if (lower == "ssh" || lower.startsWith("ssh "))
    {
        terminalPush("! SSH transport not enabled yet");
        terminalPush("local shell is active");
    }
    else
    {
        terminalPush(String("! unknown: ") + command);
        terminalPush("type help");
    }

    drawTerminal();
}

/**
 * @brief Document openTerminal.
 */
void openTerminal()
{
    currentScreen = SCREEN_TERMINAL;
    if (terminalLineCount == 0)
    {
        terminalPush(String("Platform Pocket v") + APP_VERSION);
        terminalPush("local command console ready");
        terminalPush("type help for commands");
    }
    drawTerminal();
}

/**
 * @brief Document drawBrightness.
 */
void drawBrightness()
{
    drawHeader("BRIGHTNESS", String(screenBrightness));
    M5Cardputer.Display.fillRect(12, 42, 216, 32, uiPanel);
    M5Cardputer.Display.fillRect(20, 54, 200, 9, uiPanelAlt);
    int bar = (screenBrightness * 200) / 255;
    M5Cardputer.Display.fillRect(20, 54, bar, 9, uiAccent);
    M5Cardputer.Display.setTextColor(uiMuted);
    M5Cardputer.Display.setCursor(53, 87);
    M5Cardputer.Display.print("; darker   . brighter");
    drawFooter(";/. adjust   ESC back");
}

/**
 * @brief Document openBrightness.
 */
void openBrightness()
{
    currentScreen = SCREEN_BRIGHTNESS;
    drawBrightness();
}

/**
 * @brief Document drawTheme.
 */
void drawTheme()
{
    drawHeader("THEME", "LIVE");
    M5Cardputer.Display.fillRect(12, 35, 216, 55, uiPanel);
    M5Cardputer.Display.setTextColor(uiMuted);
    M5Cardputer.Display.setCursor(82, 46);
    M5Cardputer.Display.print("ACTIVE THEME");
    M5Cardputer.Display.setTextColor(uiAccent);
    M5Cardputer.Display.setCursor(72, 66);
    M5Cardputer.Display.print(getThemeName());
    M5Cardputer.Display.setTextColor(uiMuted);
    M5Cardputer.Display.setCursor(48, 101);
    M5Cardputer.Display.print("ENTER cycles themes");
    drawFooter("ENTER cycle   ESC back");
}

/**
 * @brief Document openTheme.
 */
void openTheme()
{
    currentScreen = SCREEN_THEME;
    drawTheme();
}


/**
 * @brief Format a 32-bit IPv4 value as dotted-decimal text.
 *
 * @param value IPv4 address or mask in host-order bit layout.
 * @return Dotted-decimal representation.
 */
String formatIpv4(uint32_t value)
{
    return String((value >> 24) & 0xFF) + "." +
           String((value >> 16) & 0xFF) + "." +
           String((value >> 8) & 0xFF) + "." +
           String(value & 0xFF);
}

/**
 * @brief Convert an unsigned value to a compact binary string.
 *
 * @param value Value to convert.
 * @return Binary representation without leading zeroes.
 */
String toBinary(uint32_t value)
{
    if (value == 0)
        return "0";
    String result = "";
    while (value > 0)
    {
        result = String(value & 1U) + result;
        value >>= 1U;
    }
    return result;
}

/**
 * @brief Compute the SHA-256 digest for text entered in the local shell.
 *
 * @param value Text to hash locally on the device.
 * @return Lowercase hexadecimal SHA-256 digest.
 */
String sha256Text(const String &value)
{
    unsigned char digest[32];
    mbedtls_sha256_context ctx;
    mbedtls_sha256_init(&ctx);
    mbedtls_sha256_starts_ret(&ctx, 0);
    mbedtls_sha256_update_ret(
        &ctx,
        reinterpret_cast<const unsigned char *>(value.c_str()),
        value.length());
    mbedtls_sha256_finish_ret(&ctx, digest);
    mbedtls_sha256_free(&ctx);

    static const char hex[] = "0123456789abcdef";
    String result = "";
    result.reserve(64);
    for (unsigned char byte : digest)
    {
        result += hex[(byte >> 4) & 0x0F];
        result += hex[byte & 0x0F];
    }
    return result;
}

/**
 * @brief Show a current connected-network signal snapshot.
 */
void showSignalMonitor()
{
    if (WiFi.status() != WL_CONNECTED)
    {
        openTool("SIGNAL MONITOR", "Not connected to Wi-Fi.\n\nUse Wi-Fi Scanner for\npassive nearby RSSI values.");
        return;
    }

    int rssi = WiFi.RSSI();
    String text = "SSID: " + truncateText(WiFi.SSID(), 27);
    text += "\nRSSI: " + String(rssi) + " dBm";
    text += "\nQuality: " + String(getSignalLabel(rssi));
    text += "\nChannel: " + String(WiFi.channel());
    text += "\nIP: " + WiFi.localIP().toString();
    openTool("SIGNAL MONITOR", text);
}

/**
 * @brief Show the local network configuration for field troubleshooting.
 */
void showNetworkTools()
{
    if (WiFi.status() != WL_CONNECTED)
    {
        openTool("NETWORK TOOLS", "Connect to Wi-Fi for live\nnetwork configuration.\n\nTerminal: scan / wifi");
        return;
    }

    String text = "IP: " + WiFi.localIP().toString();
    text += "\nGW: " + WiFi.gatewayIP().toString();
    text += "\nMask: " + WiFi.subnetMask().toString();
    text += "\nDNS: " + WiFi.dnsIP().toString();
    text += "\nMAC: " + WiFi.macAddress();
    openTool("NETWORK TOOLS", text);
}

/**
 * @brief Show usage for the terminal SHA-256 tool.
 */
void showHashToolHelp()
{
    openTool("HASH TOOL", "Real SHA-256 is enabled.\n\nTerminal example:\nsha256 hello world\n\nRuns locally on-device.");
}

/**
 * @brief Show usage for defensive TCP reachability checks.
 */
void showPortCheckHelp()
{
    openTool("PORT CHECK", "TCP reachability is enabled.\nUse only hosts you own/admin.\n\nTerminal example:\nport 192.168.1.10 443");
}

/**
 * @brief Show current IP configuration shortcuts.
 */
void showIpTools()
{
    if (WiFi.status() != WL_CONNECTED)
    {
        openTool("IP TOOLS", "No active Wi-Fi lease.\n\nTerminal commands:\nscan\nwifi\nnet\ndns HOST");
        return;
    }
    showNetworkTools();
}

/**
 * @brief Show CIDR calculator usage and common masks.
 */
void showSubnetHelper()
{
    openTool("SUBNET HELPER", "CIDR calculator is live.\n\nTerminal examples:\ncidr 24\ncidr 27\n\nShows mask + addresses.");
}

/**
 * @brief Show decimal, hexadecimal, and binary conversion usage.
 */
void showBaseConverter()
{
    openTool("BASE CONVERTER", "Base converter is live.\n\nTerminal example:\nbase 255\n\nPrints decimal/hex/binary.");
}

/**
 * @brief Show internal storage and memory health information.
 */
void showStorageInfo()
{
    String text = "Flash: " + String(ESP.getFlashChipSize() / 1024UL) + " KB";
    text += "\nSketch: " + String(ESP.getSketchSize() / 1024UL) + " KB";
    text += "\nFree sketch: " + String(ESP.getFreeSketchSpace() / 1024UL) + " KB";
    text += "\nFree heap: " + String(ESP.getFreeHeap() / 1024UL) + " KB";
    text += "\nMin heap: " + String(ESP.getMinFreeHeap() / 1024UL) + " KB";
    openTool("STORAGE + MEMORY", text);
}

/**
 * @brief Document showWifiInfo.
 */
void showWifiInfo()
{
    WiFi.mode(WIFI_STA);
    String text = "MAC: " + WiFi.macAddress();
    text += "\nStatus: ";

    if (WiFi.status() == WL_CONNECTED)
    {
        text += "CONNECTED";
        text += "\nSSID: " + WiFi.SSID();
        text += "\nIP: " + WiFi.localIP().toString();
        text += "\nRSSI: " + String(WiFi.RSSI()) + " dBm";
    }
    else
    {
        text += "NOT CONNECTED";
        text += "\n\nScanner works without";
        text += "\na saved connection.";
    }

    openTool("WI-FI INFO", text);
}

/**
 * @brief Document showSecurityDashboard.
 */
void showSecurityDashboard()
{
    drawScanningScreen();
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(150);
    WiFi.scanDelete();
    wifiNetworkCount = WiFi.scanNetworks(false, true);

    int openNetworks = 0;
    int hiddenNetworks = 0;
    int duplicateNames = 0;

    for (int i = 0; i < wifiNetworkCount; i++)
    {
        if (wifiIsOpen(i))
            openNetworks++;
        if (WiFi.SSID(i).length() == 0)
            hiddenNetworks++;
        if (hasDuplicateSSID(i))
            duplicateNames++;
    }

    String text = "Networks: " + String(wifiNetworkCount);
    text += "\nOpen: " + String(openNetworks);
    text += "\nHidden: " + String(hiddenNetworks);
    text += "\nDuplicate names: " + String(duplicateNames);
    text += "\n\nPassive observations only.";

    openTool("SECURITY DASHBOARD", text);
}

/**
 * @brief Document showDeviceInfo.
 */
void showDeviceInfo()
{
    String text = String("Platform Pocket v") + APP_VERSION;
    text += "\nCPU: " + String(getCpuFrequencyMhz()) + " MHz";
    text += "\nFree heap: " + String(ESP.getFreeHeap());
    text += "\nFlash: " + String(ESP.getFlashChipSize());
    text += "\nCores: " + String(ESP.getChipCores());
    openTool("DEVICE INFO", text);
}

/**
 * @brief Document showDockerCommands.
 */
void showDockerCommands()
{
    String text = "docker ps\n  running containers";
    text += "\n\ndocker images\n  local images";
    text += "\n\ndocker logs NAME";
    text += "\n\ndocker stop NAME";
    openTool("DOCKER COMMANDS", text);
}

/**
 * @brief Document showContainerCheatsheet.
 */
void showContainerCheatsheet()
{
    String text = "Container  running instance";
    text += "\n\nImage      template";
    text += "\n\nVolume     persistent data";
    text += "\n\nPort       host -> container";
    openTool("CONTAINERS 101", text);
}

/**
 * @brief Document showComposeCheatsheet.
 */
void showComposeCheatsheet()
{
    String text = "docker compose up -d";
    text += "\n\ncompose ps";
    text += "\n\ncompose logs";
    text += "\n\ncompose down";
    openTool("COMPOSE", text);
}

/**
 * @brief Document showPasswordGenerator.
 */
void showPasswordGenerator()
{
    const char characters[] =
        "ABCDEFGHJKLMNPQRSTUVWXYZ"
        "abcdefghijkmnopqrstuvwxyz"
        "23456789"
        "!@#$%";

    randomSeed(esp_random());
    String password = "";
    for (int i = 0; i < 16; i++)
    {
        password += characters[random(0, sizeof(characters) - 1)];
    }

    openTool("PASSWORD GENERATOR", String("Generated:\n\n") + password + "\n\n16 characters");
}

/**
 * @brief Document showSystemInfo.
 */
void showSystemInfo()
{
    String text = "CPU: " + String(getCpuFrequencyMhz()) + " MHz";
    text += "\nHeap: " + String(ESP.getFreeHeap());
    text += "\nFlash: " + String(ESP.getFlashChipSize());
    text += "\nUptime: " + String(millis() / 1000UL) + " sec";
    text += "\nSDK: " + String(ESP.getSdkVersion());
    openTool("SYSTEM INFO", text);
}

/**
 * @brief Document showAbout.
 */
void showAbout()
{
    String text = String("Platform Pocket v") + APP_VERSION;
    text += "\n\nESP32-S3 handheld toolkit";
    text += "\nfor networking, platform";
    text += "\nwork and quick diagnostics.";
    text += "\n\nPocket workstation + SD.";
    openTool("ABOUT", text);
}

/**
 * @brief Document executeSelectedTool.
 */
void executeSelectedTool()
{
    int option = selectedSubItem[selectedMainItem];

    if (selectedMainItem == MAIN_NETWORK)
    {
        switch (option)
        {
        case 0:
            scanWifiNetworks();
            drawWifiResults();
            break;
        case 1:
            showWifiInfo();
            break;
        case 2:
            showSignalMonitor();
            break;
        case 3:
            openTool("DNS LOOKUP", "DNS resolver is live.\n\nTerminal example:\ndns example.com\n\nRequires connected Wi-Fi.");
            break;
        case 4:
            showNetworkTools();
            break;
        }
    }
    else if (selectedMainItem == MAIN_SECURITY)
    {
        switch (option)
        {
        case 0:
            showSecurityDashboard();
            break;
        case 1:
            openTool("WI-FI OBSERVATIONS", "Passive checks:\n\n- Open networks\n- Hidden SSIDs\n- Duplicate SSIDs\n- Signal strength\n\nNo attacker claims.");
            break;
        case 2:
            showDeviceInfo();
            break;
        case 3:
            showHashToolHelp();
            break;
        case 4:
            showPortCheckHelp();
            break;
        }
    }
    else if (selectedMainItem == MAIN_DOCKER)
    {
        switch (option)
        {
        case 0:
            showDockerCommands();
            break;
        case 1:
            showContainerCheatsheet();
            break;
        case 2:
            showComposeCheatsheet();
            break;
        case 3:
            openTool("REMOTE HOST", "Remote Linux control is the\nnext terminal milestone.\n\nCardputer's local shell is\nalready available now.");
            break;
        }
    }
    else if (selectedMainItem == MAIN_NOTES)
    {
        switch (option)
        {
        case 0:
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
            break;
        }
    }
    else if (selectedMainItem == MAIN_TOOLS)
    {
        switch (option)
        {
        case 0:
            showIpTools();
            break;
        case 1:
            showSubnetHelper();
            break;
        case 2:
            showBaseConverter();
            break;
        case 3:
            showPasswordGenerator();
            break;
        case 4:
            showSystemInfo();
            break;
        }
    }
    else if (selectedMainItem == MAIN_SETTINGS)
    {
        switch (option)
        {
        case 0:
            openBrightness();
            break;
        case 1:
            openTheme();
            break;
        case 2:
            openTool("WI-FI SETTINGS", "Saved trusted Wi-Fi\nprofiles are planned.");
            break;
        case 3:
            openTool("STORAGE", PocketStorage::statusText() + "\n\n" + String("Heap: ") + ESP.getFreeHeap());
            break;
        case 4:
            showAbout();
            break;
        }
    }
}

/**
 * @brief Document setup.
 */
void setup()
{
    auto cfg = M5.config();
    M5Cardputer.begin(cfg, true);
    M5Cardputer.Display.setRotation(1);
    M5Cardputer.Display.setBrightness(screenBrightness);
    PocketStorage::begin();
    PocketWorkstation::begin();
    applyTheme();
    drawMainMenu();
}

/**
 * @brief Document loop.
 */
void loop()
{
    M5Cardputer.update();

    if (!M5Cardputer.Keyboard.isChange() || !M5Cardputer.Keyboard.isPressed())
    {
        return;
    }

    Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();

    if (currentScreen == SCREEN_EDITOR)
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

    if (currentScreen == SCREEN_EDITOR)
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

    if (currentScreen == SCREEN_EDITOR)
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

    if (currentScreen == SCREEN_TERMINAL)
    {
        if (status.esc)
        {
            currentScreen = SCREEN_MAIN;
            drawMainMenu();
            return;
        }

        if ((status.del || status.backspace) && terminalInput.length() > 0)
        {
            terminalInput.remove(terminalInput.length() - 1);
        }

        for (auto key : status.word)
        {
            if (key >= 32 && key <= 126 && terminalInput.length() < 48)
            {
                terminalInput += key;
            }
        }

        if (status.enter)
        {
            runTerminalCommand();
        }
        else
        {
            drawTerminal();
        }
        return;
    }

    if (currentScreen == SCREEN_BRIGHTNESS)
    {
        if (status.esc)
        {
            currentScreen = SCREEN_SECTION_MENU;
            drawSectionMenu();
            return;
        }

        if (M5Cardputer.Keyboard.isKeyPressed(';'))
            screenBrightness -= 16;
        if (M5Cardputer.Keyboard.isKeyPressed('.'))
            screenBrightness += 16;
        if (screenBrightness < 16)
            screenBrightness = 16;
        if (screenBrightness > 255)
            screenBrightness = 255;
        M5Cardputer.Display.setBrightness(screenBrightness);
        drawBrightness();
        return;
    }

    if (currentScreen == SCREEN_THEME)
    {
        if (status.esc)
        {
            currentScreen = SCREEN_SECTION_MENU;
            drawSectionMenu();
            return;
        }

        if (status.enter)
        {
            themeIndex = (themeIndex + 1) % 3;
            applyTheme();
            drawTheme();
        }
        return;
    }

    if (currentScreen == SCREEN_MAIN)
    {
        if (M5Cardputer.Keyboard.isKeyPressed(';'))
        {
            selectedMainItem--;
            if (selectedMainItem < 0)
                selectedMainItem = MAIN_COUNT - 1;
            drawMainMenu();
        }
        else if (M5Cardputer.Keyboard.isKeyPressed('.'))
        {
            selectedMainItem++;
            if (selectedMainItem >= MAIN_COUNT)
                selectedMainItem = 0;
            drawMainMenu();
        }
        else if (status.enter)
        {
            if (selectedMainItem == MAIN_TERMINAL)
            {
                openTerminal();
            }
            else
            {
                currentScreen = SCREEN_SECTION_MENU;
                drawSectionMenu();
            }
        }
        return;
    }

    if (currentScreen == SCREEN_SECTION_MENU)
    {
        int count = getCurrentSubMenuCount();
        int &selection = selectedSubItem[selectedMainItem];

        if (status.esc)
        {
            currentScreen = SCREEN_MAIN;
            drawMainMenu();
        }
        else if (M5Cardputer.Keyboard.isKeyPressed(';'))
        {
            selection--;
            if (selection < 0)
                selection = count - 1;
            drawSectionMenu();
        }
        else if (M5Cardputer.Keyboard.isKeyPressed('.'))
        {
            selection++;
            if (selection >= count)
                selection = 0;
            drawSectionMenu();
        }
        else if (status.enter)
        {
            executeSelectedTool();
        }
        return;
    }

    if (currentScreen == SCREEN_TOOL)
    {
        if (status.esc)
        {
            currentScreen = SCREEN_SECTION_MENU;
            drawSectionMenu();
        }
        return;
    }

    if (currentScreen == SCREEN_WIFI_SCAN)
    {
        if (status.esc)
        {
            currentScreen = SCREEN_SECTION_MENU;
            drawSectionMenu();
            return;
        }

        if (M5Cardputer.Keyboard.isKeyPressed(';') && wifiNetworkCount > 0)
        {
            selectedWifiNetwork--;
            if (selectedWifiNetwork < 0)
                selectedWifiNetwork = wifiNetworkCount - 1;
            if (selectedWifiNetwork < wifiScrollOffset)
                wifiScrollOffset = selectedWifiNetwork;
            if (selectedWifiNetwork >= wifiScrollOffset + wifiRowsVisible)
                wifiScrollOffset = selectedWifiNetwork - wifiRowsVisible + 1;
            drawWifiResults();
        }
        else if (M5Cardputer.Keyboard.isKeyPressed('.') && wifiNetworkCount > 0)
        {
            selectedWifiNetwork++;
            if (selectedWifiNetwork >= wifiNetworkCount)
                selectedWifiNetwork = 0;
            if (selectedWifiNetwork >= wifiScrollOffset + wifiRowsVisible)
                wifiScrollOffset++;
            if (selectedWifiNetwork < wifiScrollOffset)
                wifiScrollOffset = 0;
            drawWifiResults();
        }
        else if (status.enter)
        {
            if (wifiNetworkCount > 0)
            {
                currentScreen = SCREEN_WIFI_DETAILS;
                drawWifiDetails();
            }
            else
            {
                scanWifiNetworks();
                drawWifiResults();
            }
        }
        return;
    }

    if (currentScreen == SCREEN_WIFI_DETAILS)
    {
        if (status.esc)
        {
            currentScreen = SCREEN_WIFI_SCAN;
            drawWifiResults();
        }
    }
}
