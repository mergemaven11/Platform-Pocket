#include <M5Cardputer.h>
#include <WiFi.h>
#include <esp_system.h>

// ============================================================
// PLATFORM POCKET v0.5
// A compact handheld platform / network toolkit for Cardputer ADV.
// ============================================================

static const char *APP_VERSION = "0.5";

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
    SCREEN_THEME
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
    "View Notes",
    "New Note",
    "Delete Note",
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

const char *getThemeName()
{
    if (themeIndex == 0)
        return "MIDNIGHT CYAN";
    if (themeIndex == 1)
        return "MATRIX GREEN";
    return "AMBER OPS";
}

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

void drawStatusHeader()
{
    drawHeader("PLATFORM POCKET", String("v") + APP_VERSION);

    uint16_t dotColor = WiFi.status() == WL_CONNECTED ? uiSuccess : uiMuted;
    M5Cardputer.Display.fillRect(174, 7, 5, 5, dotColor);
    M5Cardputer.Display.setTextColor(uiMuted);
    M5Cardputer.Display.setCursor(182, 6);
    M5Cardputer.Display.print(WiFi.status() == WL_CONNECTED ? "NET" : "OFF");
}

void drawFooter(const String &hint)
{
    M5Cardputer.Display.fillRect(0, 121, 240, 14, uiHeader);
    M5Cardputer.Display.drawFastHLine(0, 120, 240, uiPanelAlt);
    M5Cardputer.Display.setTextSize(1);
    M5Cardputer.Display.setTextColor(uiMuted);
    M5Cardputer.Display.setCursor(6, 125);
    M5Cardputer.Display.print(hint);
}

String truncateText(const String &value, int maxChars)
{
    if ((int)value.length() <= maxChars)
        return value;
    if (maxChars <= 3)
        return value.substring(0, maxChars);
    return value.substring(0, maxChars - 3) + "...";
}

const char *getSectionTitle()
{
    return mainMenuItems[selectedMainItem];
}

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

bool wifiIsOpen(int index)
{
    return WiFi.encryptionType(index) == WIFI_AUTH_OPEN;
}

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

void openTool(const String &title, const String &message)
{
    toolTitle = title;
    toolMessage = message;
    currentScreen = SCREEN_TOOL;
    drawToolPage();
}

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

void scanWifiNetworks()
{
    drawScanningScreen();
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(150);
    WiFi.scanDelete();
    wifiNetworkCount = WiFi.scanNetworks(false, true);
    selectedWifiNetwork = 0;
    wifiScrollOffset = 0;
    currentScreen = SCREEN_WIFI_SCAN;
}

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

void terminalClear()
{
    terminalLineCount = 0;
    terminalInput = "";
    for (int i = 0; i < 6; i++)
        terminalLines[i] = "";
}

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

    lastTerminalCommand = command;
    terminalPush(String("> ") + command);

    String lower = command;
    lower.toLowerCase();

    if (lower == "help" || lower == "?")
    {
        terminalPush("help clear wifi scan ip");
        terminalPush("sysinfo uptime docker history");
        terminalPush("version ssh");
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
        terminalPush(lastTerminalCommand.length() ? String("last: ") + lastTerminalCommand : "history empty");
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

void openBrightness()
{
    currentScreen = SCREEN_BRIGHTNESS;
    drawBrightness();
}

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

void openTheme()
{
    currentScreen = SCREEN_THEME;
    drawTheme();
}

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

void showDeviceInfo()
{
    String text = String("Platform Pocket v") + APP_VERSION;
    text += "\nCPU: " + String(getCpuFrequencyMhz()) + " MHz";
    text += "\nFree heap: " + String(ESP.getFreeHeap());
    text += "\nFlash: " + String(ESP.getFlashChipSize());
    text += "\nCores: " + String(ESP.getChipCores());
    openTool("DEVICE INFO", text);
}

void showDockerCommands()
{
    String text = "docker ps\n  running containers";
    text += "\n\ndocker images\n  local images";
    text += "\n\ndocker logs NAME";
    text += "\n\ndocker stop NAME";
    openTool("DOCKER COMMANDS", text);
}

void showContainerCheatsheet()
{
    String text = "Container  running instance";
    text += "\n\nImage      template";
    text += "\n\nVolume     persistent data";
    text += "\n\nPort       host -> container";
    openTool("CONTAINERS 101", text);
}

void showComposeCheatsheet()
{
    String text = "docker compose up -d";
    text += "\n\ncompose ps";
    text += "\n\ncompose logs";
    text += "\n\ncompose down";
    openTool("COMPOSE", text);
}

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

void showSystemInfo()
{
    String text = "CPU: " + String(getCpuFrequencyMhz()) + " MHz";
    text += "\nHeap: " + String(ESP.getFreeHeap());
    text += "\nFlash: " + String(ESP.getFlashChipSize());
    text += "\nUptime: " + String(millis() / 1000UL) + " sec";
    text += "\nSDK: " + String(ESP.getSdkVersion());
    openTool("SYSTEM INFO", text);
}

void showAbout()
{
    String text = String("Platform Pocket v") + APP_VERSION;
    text += "\n\nESP32-S3 handheld toolkit";
    text += "\nfor networking, platform";
    text += "\nwork and quick diagnostics.";
    text += "\n\nUI + terminal refresh.";
    openTool("ABOUT", text);
}

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
            openTool("SIGNAL MONITOR", "Live RSSI graph is queued.\n\nWi-Fi scanner already shows\nper-network signal strength.");
            break;
        case 3:
            openTool("DNS LOOKUP", "Hostname input is queued.\n\nRequires an active Wi-Fi\nconnection to resolve DNS.");
            break;
        case 4:
            openTool("NETWORK TOOLS", "Available in Terminal now:\n\nscan\nip\nwifi\nsysinfo\nuptime");
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
            openTool("HASH TOOL", "SHA-256 text/file hashing\nis planned for a later build.");
            break;
        case 4:
            openTool("PORT CHECK", "Defensive port checks for\nhosts you own/administer\nare planned.");
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
            openTool("VIEW NOTES", "SD-backed notes are queued.\n\nThe redesigned shell now\nprovides keyboard input.");
            break;
        case 1:
            openTool("NEW NOTE", "Note editor is queued for\nthe next storage pass.");
            break;
        case 2:
            openTool("DELETE NOTE", "Saved note management is\nqueued with SD support.");
            break;
        case 3:
            openTool("SD STORAGE", "SD capacity, free space\nand note files are queued.");
            break;
        }
    }
    else if (selectedMainItem == MAIN_TOOLS)
    {
        switch (option)
        {
        case 0:
            openTool("IP TOOLS", "Terminal commands now include:\n\nip\nwifi\nscan\n\nMore parsers are queued.");
            break;
        case 1:
            openTool("SUBNET HELPER", "/24 = 255.255.255.0\n/16 = 255.255.0.0\n/8  = 255.0.0.0\n\nCalculator is queued.");
            break;
        case 2:
            openTool("BASE CONVERTER", "Decimal / binary / hex\nconversion is queued.");
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
            openTool("STORAGE", "Internal flash + SD status\nwill live here.");
            break;
        case 4:
            showAbout();
            break;
        }
    }
}

void setup()
{
    auto cfg = M5.config();
    M5Cardputer.begin(cfg, true);
    M5Cardputer.Display.setRotation(1);
    M5Cardputer.Display.setBrightness(screenBrightness);
    applyTheme();
    drawMainMenu();
}

void loop()
{
    M5Cardputer.update();

    if (!M5Cardputer.Keyboard.isChange() || !M5Cardputer.Keyboard.isPressed())
    {
        return;
    }

    Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();

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
