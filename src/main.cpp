#include <M5Cardputer.h>
#include <WiFi.h>
#include <esp_system.h>

// ============================================================
// PLATFORM POCKET v0.4
// ============================================================
//
// PURPOSE
// -------
// Platform Pocket is becoming a small handheld toolkit for:
//
// - Networking
// - Security / diagnostics
// - Docker learning
// - Notes
// - Quick utilities
// - Device settings
//
// ------------------------------------------------------------
// CONTROLS
// ------------------------------------------------------------
//
// ;        = Move Up
// .        = Move Down
// ENTER    = Select
// Fn + `   = Back / Escape
//
// ------------------------------------------------------------
// LEARNING NOTE
// ------------------------------------------------------------
//
// This program is organized into:
//
// 1. DATA
//    Menu labels and variables.
//
// 2. STATE
//    Variables that remember where the user is.
//
// 3. DRAW FUNCTIONS
//    Functions that put things on the screen.
//
// 4. TOOL FUNCTIONS
//    Functions that actually perform work.
//
// 5. INPUT HANDLING
//    loop() watches the keyboard and reacts.
//
// This separation becomes VERY important as programs grow.
//
// ============================================================

// ============================================================
// MAIN MENU IDENTIFIERS
// ============================================================
//
// An enum lets us give readable names to numbers.
//
// Internally:
//
// MAIN_NETWORK     = 0
// MAIN_SECURITY    = 1
// MAIN_DOCKER      = 2
//
// But instead of writing:
//
//     if (selectedMainItem == 2)
//
// we can write:
//
//     if (selectedMainItem == MAIN_DOCKER)
//
// That's much easier to understand.

enum MainMenuItem
{
    MAIN_NETWORK,
    MAIN_SECURITY,
    MAIN_DOCKER,
    MAIN_NOTES,
    MAIN_QUICK_TOOLS,
    MAIN_SETTINGS,

    // MAIN_COUNT becomes the total number of items.
    MAIN_COUNT
};

// ============================================================
// SCREEN STATES
// ============================================================
//
// Another enum.
//
// This one describes WHICH SCREEN is currently visible.
//
// This is the beginning of a "state machine".
//
// A state machine simply means:
//
// "The program behaves differently depending on
//  what state it is currently in."

enum ScreenState
{
    SCREEN_MAIN,
    SCREEN_SECTION_MENU,
    SCREEN_TOOL,
    SCREEN_WIFI_SCAN,
    SCREEN_WIFI_DETAILS
};

// Current screen.
ScreenState currentScreen = SCREEN_MAIN;

// ============================================================
// MAIN MENU DATA
// ============================================================

const char *mainMenuItems[] = {
    "Network",
    "Security",
    "Docker",
    "Notes",
    "Quick Tools",
    "Settings"};

// Which main menu item is selected.
int selectedMainItem = 0;

// ============================================================
// NETWORK MENU
// ============================================================

const char *networkMenu[] = {
    "Wi-Fi Scanner",
    "Wi-Fi Info",
    "Signal Monitor",
    "DNS Lookup",
    "Network Tools"};

const int networkMenuCount = 5;

// ============================================================
// SECURITY MENU
// ============================================================

const char *securityMenu[] = {
    "Security Dashboard",
    "Wi-Fi Observations",
    "Device Info",
    "Hash Tool",
    "Port Check"};

const int securityMenuCount = 5;

// ============================================================
// DOCKER MENU
// ============================================================

const char *dockerMenu[] = {
    "Docker Commands",
    "Container Cheatsheet",
    "Compose Cheatsheet",
    "Remote Host"};

const int dockerMenuCount = 4;

// ============================================================
// NOTES MENU
// ============================================================

const char *notesMenu[] = {
    "View Notes",
    "New Note",
    "Delete Note",
    "SD Storage"};

const int notesMenuCount = 4;

// ============================================================
// QUICK TOOLS MENU
// ============================================================

const char *quickToolsMenu[] = {
    "IP Tools",
    "Subnet Helper",
    "Base Converter",
    "Password Generator",
    "System Info"};

const int quickToolsMenuCount = 5;

// ============================================================
// SETTINGS MENU
// ============================================================

const char *settingsMenu[] = {
    "Brightness",
    "Theme",
    "Wi-Fi Settings",
    "Storage",
    "About"};

const int settingsMenuCount = 5;

// ============================================================
// SUBMENU STATE
// ============================================================
//
// Each main section needs to remember its own selection.
//
// Example:
//
// selectedSubItem[MAIN_NETWORK]
// selectedSubItem[MAIN_SECURITY]
//
// Because enums are numbers underneath,
// they can also be used as array indexes.

int selectedSubItem[MAIN_COUNT] = {
    0,
    0,
    0,
    0,
    0,
    0};

// ============================================================
// WI-FI SCANNER STATE
// ============================================================

// How many networks were discovered.
int wifiNetworkCount = 0;

// Which network is highlighted.
int selectedWifiNetwork = 0;

// First network currently visible.
int wifiScrollOffset = 0;

// Number of Wi-Fi results shown at once.
const int wifiRowsVisible = 4;

// ============================================================
// SETTINGS STATE
// ============================================================

// Display brightness.
//
// Typical range:
//
// 0   = dark
// 255 = maximum
//
// We start at 128.

int screenBrightness = 128;

// ============================================================
// GENERIC TOOL PAGE
// ============================================================
//
// Some tools aren't implemented yet.
//
// Rather than duplicate the same display code
// twenty times, we store a title and message here.

String toolTitle = "";
String toolMessage = "";

// ============================================================
// HELPER: GET SECTION TITLE
// ============================================================

const char *getSectionTitle()
{
    return mainMenuItems[selectedMainItem];
}

// ============================================================
// HELPER: GET CURRENT SUBMENU
// ============================================================
//
// This function returns the correct menu array.
//
// Notice the return type:
//
//     const char**
//
// That means:
//
// "Return a pointer to an array of text pointers."
//
// Don't worry if that looks weird right now.
// Pointer syntax is one of the stranger parts of C++.

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

    case MAIN_QUICK_TOOLS:
        return quickToolsMenu;

    case MAIN_SETTINGS:
        return settingsMenu;
    }

    return networkMenu;
}

// ============================================================
// HELPER: GET CURRENT SUBMENU COUNT
// ============================================================

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

    case MAIN_QUICK_TOOLS:
        return quickToolsMenuCount;

    case MAIN_SETTINGS:
        return settingsMenuCount;
    }

    return 0;
}

// ============================================================
// SIGNAL STRENGTH LABEL
// ============================================================
//
// RSSI = Received Signal Strength Indicator.
//
// Examples:
//
// -30 dBm = extremely strong
// -50 dBm = strong
// -70 dBm = usable
// -90 dBm = very weak

const char *getSignalLabel(int rssi)
{
    if (rssi >= -50)
    {
        return "STRONG";
    }

    if (rssi >= -65)
    {
        return "GOOD";
    }

    if (rssi >= -75)
    {
        return "FAIR";
    }

    return "WEAK";
}

// ============================================================
// WI-FI SECURITY CHECK
// ============================================================

bool wifiIsOpen(int index)
{
    return WiFi.encryptionType(index) == WIFI_AUTH_OPEN;
}

// ============================================================
// DUPLICATE SSID CHECK
// ============================================================
//
// SSID = Wi-Fi name.
//
// Multiple access points can legitimately use
// the same SSID.
//
// Examples:
//
// - Mesh networks
// - Hotels
// - Offices
// - Schools
//
// So DUPLICATE DOES NOT MEAN ATTACKER.
//
// We only report it as an observation.

bool hasDuplicateSSID(int index)
{
    String target = WiFi.SSID(index);

    if (target.length() == 0)
    {
        return false;
    }

    for (int i = 0; i < wifiNetworkCount; i++)
    {
        if (i == index)
        {
            continue;
        }

        if (WiFi.SSID(i) == target)
        {
            return true;
        }
    }

    return false;
}

// ============================================================
// DRAW MAIN MENU
// ============================================================

void drawMainMenu()
{
    M5Cardputer.Display.fillScreen(BLACK);

    // TITLE
    M5Cardputer.Display.setTextColor(GREEN);
    M5Cardputer.Display.setTextSize(2);
    M5Cardputer.Display.setCursor(10, 10);

    M5Cardputer.Display.println("PLATFORM POCKET");

    // VERSION
    M5Cardputer.Display.setTextColor(WHITE);
    M5Cardputer.Display.setTextSize(1);
    M5Cardputer.Display.setCursor(10, 40);

    M5Cardputer.Display.println("v0.4");

    // MENU
    M5Cardputer.Display.setCursor(10, 58);

    for (int i = 0; i < MAIN_COUNT; i++)
    {
        if (i == selectedMainItem)
        {
            M5Cardputer.Display.print("> ");
        }
        else
        {
            M5Cardputer.Display.print("  ");
        }

        M5Cardputer.Display.println(mainMenuItems[i]);
    }
}

// ============================================================
// DRAW SECTION MENU
// ============================================================
//
// This ONE function can display:
//
// Network
// Security
// Docker
// Notes
// Quick Tools
// Settings
//
// That's better than writing six almost-identical functions.

void drawSectionMenu()
{
    M5Cardputer.Display.fillScreen(BLACK);

    // Section title
    M5Cardputer.Display.setTextColor(GREEN);
    M5Cardputer.Display.setTextSize(2);
    M5Cardputer.Display.setCursor(8, 8);

    M5Cardputer.Display.println(getSectionTitle());

    // Get menu array and count.
    const char **menu = getCurrentSubMenu();

    int count = getCurrentSubMenuCount();

    int selected = selectedSubItem[selectedMainItem];

    M5Cardputer.Display.setTextColor(WHITE);
    M5Cardputer.Display.setTextSize(1);
    M5Cardputer.Display.setCursor(8, 38);

    for (int i = 0; i < count; i++)
    {
        if (i == selected)
        {
            M5Cardputer.Display.print("> ");
        }
        else
        {
            M5Cardputer.Display.print("  ");
        }

        M5Cardputer.Display.println(menu[i]);
    }

    M5Cardputer.Display.setCursor(8, 118);

    M5Cardputer.Display.print("ENTER Select   ESC Back");
}

// ============================================================
// DRAW GENERIC TOOL PAGE
// ============================================================

void drawToolPage()
{
    M5Cardputer.Display.fillScreen(BLACK);

    M5Cardputer.Display.setTextColor(GREEN);
    M5Cardputer.Display.setTextSize(1);
    M5Cardputer.Display.setCursor(6, 6);

    M5Cardputer.Display.println(toolTitle);

    M5Cardputer.Display.setTextColor(WHITE);
    M5Cardputer.Display.setCursor(6, 25);

    M5Cardputer.Display.println(toolMessage);

    M5Cardputer.Display.setCursor(6, 118);

    M5Cardputer.Display.println("ESC Back");
}

// ============================================================
// OPEN GENERIC TOOL
// ============================================================

void openTool(String title, String message)
{
    toolTitle = title;
    toolMessage = message;

    currentScreen = SCREEN_TOOL;

    drawToolPage();
}

// ============================================================
// WI-FI SCANNING SCREEN
// ============================================================

void drawScanningScreen()
{
    M5Cardputer.Display.fillScreen(BLACK);

    M5Cardputer.Display.setTextColor(GREEN);
    M5Cardputer.Display.setTextSize(2);
    M5Cardputer.Display.setCursor(10, 20);

    M5Cardputer.Display.println("SCANNING...");

    M5Cardputer.Display.setTextColor(WHITE);
    M5Cardputer.Display.setTextSize(1);
    M5Cardputer.Display.setCursor(10, 55);

    M5Cardputer.Display.println("Listening for nearby");
    M5Cardputer.Display.println("Wi-Fi access points.");
}

// ============================================================
// SCAN WI-FI
// ============================================================

void scanWifiNetworks()
{
    drawScanningScreen();

    // Station mode enables the radio as a client.
    //
    // We do not need to connect to a network
    // just to scan for access points.

    WiFi.mode(WIFI_STA);

    // Disconnect THIS Cardputer if connected.
    WiFi.disconnect();

    delay(200);

    // Delete old scan results.
    WiFi.scanDelete();

    // --------------------------------------------------------
    // scanNetworks(false, true)
    // --------------------------------------------------------
    //
    // false:
    // Perform the scan synchronously.
    //
    // That means the function waits until scanning finishes.
    //
    // true:
    // Also try to include hidden SSIDs.

    wifiNetworkCount = WiFi.scanNetworks(false, true);

    selectedWifiNetwork = 0;

    wifiScrollOffset = 0;

    currentScreen = SCREEN_WIFI_SCAN;
}

// ============================================================
// DRAW WI-FI RESULTS
// ============================================================

void drawWifiResults()
{
    M5Cardputer.Display.fillScreen(BLACK);

    M5Cardputer.Display.setTextColor(GREEN);
    M5Cardputer.Display.setTextSize(1);
    M5Cardputer.Display.setCursor(4, 4);

    M5Cardputer.Display.print("WI-FI SCANNER | ");

    M5Cardputer.Display.print(wifiNetworkCount);

    M5Cardputer.Display.println(" FOUND");

    if (wifiNetworkCount <= 0)
    {
        M5Cardputer.Display.setTextColor(WHITE);
        M5Cardputer.Display.setCursor(5, 30);

        M5Cardputer.Display.println("No networks found.");
        M5Cardputer.Display.println();
        M5Cardputer.Display.println("ENTER = Scan again");
        M5Cardputer.Display.println("ESC   = Back");

        return;
    }

    M5Cardputer.Display.setCursor(4, 20);

    int finalRow = wifiScrollOffset + wifiRowsVisible;

    if (finalRow > wifiNetworkCount)
    {
        finalRow = wifiNetworkCount;
    }

    for (int i = wifiScrollOffset; i < finalRow; i++)
    {
        String ssid = WiFi.SSID(i);

        if (ssid.length() == 0)
        {
            ssid = "<hidden>";
        }

        // Shorten very long network names.
        if (ssid.length() > 18)
        {
            ssid = ssid.substring(0, 18);
        }

        if (i == selectedWifiNetwork)
        {
            M5Cardputer.Display.setTextColor(GREEN);
            M5Cardputer.Display.print("> ");
        }
        else
        {
            M5Cardputer.Display.setTextColor(WHITE);
            M5Cardputer.Display.print("  ");
        }

        M5Cardputer.Display.print(ssid);

        if (wifiIsOpen(i))
        {
            M5Cardputer.Display.print(" [OPEN]");
        }
        else if (hasDuplicateSSID(i))
        {
            M5Cardputer.Display.print(" [DUP]");
        }

        M5Cardputer.Display.println();

        M5Cardputer.Display.setTextColor(WHITE);

        M5Cardputer.Display.print("   ");

        M5Cardputer.Display.print(WiFi.RSSI(i));

        M5Cardputer.Display.print(" dBm CH");

        M5Cardputer.Display.println(WiFi.channel(i));
    }

    M5Cardputer.Display.setCursor(4, 118);

    M5Cardputer.Display.print("ENTER Details  ESC Back");
}

// ============================================================
// DRAW WI-FI DETAILS
// ============================================================

void drawWifiDetails()
{
    if (wifiNetworkCount <= 0)
    {
        return;
    }

    M5Cardputer.Display.fillScreen(BLACK);

    M5Cardputer.Display.setTextColor(GREEN);
    M5Cardputer.Display.setTextSize(1);
    M5Cardputer.Display.setCursor(5, 5);

    M5Cardputer.Display.println("NETWORK DETAILS");

    M5Cardputer.Display.setTextColor(WHITE);
    M5Cardputer.Display.setCursor(5, 20);

    String ssid = WiFi.SSID(selectedWifiNetwork);

    if (ssid.length() == 0)
    {
        ssid = "<hidden>";
    }

    M5Cardputer.Display.print("SSID: ");

    M5Cardputer.Display.println(ssid);

    M5Cardputer.Display.print("BSSID: ");

    M5Cardputer.Display.println(
        WiFi.BSSIDstr(selectedWifiNetwork));

    int rssi = WiFi.RSSI(selectedWifiNetwork);

    M5Cardputer.Display.print("Signal: ");

    M5Cardputer.Display.print(rssi);

    M5Cardputer.Display.print(" ");

    M5Cardputer.Display.println(getSignalLabel(rssi));

    M5Cardputer.Display.print("Channel: ");

    M5Cardputer.Display.println(
        WiFi.channel(selectedWifiNetwork));

    M5Cardputer.Display.print("Security: ");

    if (wifiIsOpen(selectedWifiNetwork))
    {
        M5Cardputer.Display.println("OPEN");
    }
    else
    {
        M5Cardputer.Display.println("SECURED");
    }

    M5Cardputer.Display.print("Duplicate SSID: ");

    if (hasDuplicateSSID(selectedWifiNetwork))
    {
        M5Cardputer.Display.println("YES");
    }
    else
    {
        M5Cardputer.Display.println("NO");
    }

    M5Cardputer.Display.setCursor(5, 118);

    M5Cardputer.Display.println("ESC Back");
}

// ============================================================
// WI-FI INFO
// ============================================================

void showWifiInfo()
{
    WiFi.mode(WIFI_STA);

    String text = "";

    text += "MAC:\n";
    text += WiFi.macAddress();

    text += "\n\nStatus: ";

    if (WiFi.status() == WL_CONNECTED)
    {
        text += "CONNECTED";

        text += "\nSSID: ";
        text += WiFi.SSID();

        text += "\nIP: ";
        text += WiFi.localIP().toString();

        text += "\nRSSI: ";
        text += String(WiFi.RSSI());
        text += " dBm";
    }
    else
    {
        text += "NOT CONNECTED";

        text += "\n\nScanning does not require";
        text += "\na Wi-Fi connection.";
    }

    openTool(
        "WI-FI INFO",
        text);
}

// ============================================================
// SECURITY DASHBOARD
// ============================================================
//
// This performs a Wi-Fi scan and creates a SUMMARY.
//
// Notice:
//
// We say "observations".
//
// We do NOT claim:
//
// "This network is malicious."
//
// Passive scan information alone cannot reliably
// prove that.

void showSecurityDashboard()
{
    drawScanningScreen();

    WiFi.mode(WIFI_STA);

    WiFi.disconnect();

    delay(200);

    WiFi.scanDelete();

    wifiNetworkCount = WiFi.scanNetworks(false, true);

    int openNetworks = 0;
    int hiddenNetworks = 0;
    int duplicateNames = 0;

    for (int i = 0; i < wifiNetworkCount; i++)
    {
        if (wifiIsOpen(i))
        {
            openNetworks++;
        }

        if (WiFi.SSID(i).length() == 0)
        {
            hiddenNetworks++;
        }

        if (hasDuplicateSSID(i))
        {
            duplicateNames++;
        }
    }

    String text = "";

    text += "Networks found: ";
    text += String(wifiNetworkCount);

    text += "\nOpen: ";
    text += String(openNetworks);

    text += "\nHidden: ";
    text += String(hiddenNetworks);

    text += "\nDuplicate SSIDs: ";
    text += String(duplicateNames);

    text += "\n\nObservations only.";
    text += "\nNot proof of malicious";
    text += "\nactivity.";

    openTool(
        "SECURITY DASHBOARD",
        text);
}

// ============================================================
// DEVICE INFO
// ============================================================

void showDeviceInfo()
{
    String text = "";

    text += "Platform Pocket v0.4";

    text += "\n\nCPU MHz: ";
    text += String(getCpuFrequencyMhz());

    text += "\nFree Heap: ";
    text += String(ESP.getFreeHeap());

    text += "\nFlash: ";
    text += String(ESP.getFlashChipSize());

    text += "\nChip cores: ";
    text += String(ESP.getChipCores());

    openTool(
        "DEVICE INFO",
        text);
}

// ============================================================
// DOCKER COMMANDS
// ============================================================

void showDockerCommands()
{
    String text = "";

    text += "docker ps";
    text += "\n  Running containers";

    text += "\n\ndocker images";
    text += "\n  Local images";

    text += "\n\ndocker logs NAME";
    text += "\n  Container logs";

    text += "\n\ndocker stop NAME";

    openTool(
        "DOCKER COMMANDS",
        text);
}

// ============================================================
// CONTAINER CHEATSHEET
// ============================================================

void showContainerCheatsheet()
{
    String text = "";

    text += "Container";
    text += "\n  Running instance";

    text += "\n\nImage";
    text += "\n  Container template";

    text += "\n\nVolume";
    text += "\n  Persistent storage";

    text += "\n\nPort";
    text += "\n  Host -> container";

    openTool(
        "CONTAINERS 101",
        text);
}

// ============================================================
// COMPOSE CHEATSHEET
// ============================================================

void showComposeCheatsheet()
{
    String text = "";

    text += "docker compose up -d";

    text += "\n\ncompose ps";

    text += "\n\ncompose logs";

    text += "\n\ncompose down";

    text += "\n\ncompose.yml describes";
    text += "\nmulti-container apps.";

    openTool(
        "COMPOSE CHEATSHEET",
        text);
}

// ============================================================
// PASSWORD GENERATOR
// ============================================================
//
// This demonstrates:
//
// - arrays
// - random numbers
// - loops
// - string building

void showPasswordGenerator()
{
    const char characters[] =
        "ABCDEFGHJKLMNPQRSTUVWXYZ"
        "abcdefghijkmnopqrstuvwxyz"
        "23456789"
        "!@#$%";

    const int passwordLength = 16;

    String password = "";

    // Seed Arduino's random number generator
    // using a hardware-generated ESP32 random value.

    randomSeed(esp_random());

    for (int i = 0; i < passwordLength; i++)
    {
        int randomIndex =
            random(0, sizeof(characters) - 1);

        password += characters[randomIndex];
    }

    String text = "";

    text += "Generated password:\n\n";

    text += password;

    text += "\n\n16 characters";

    openTool(
        "PASSWORD GENERATOR",
        text);
}

// ============================================================
// SYSTEM INFO
// ============================================================

void showSystemInfo()
{
    String text = "";

    text += "CPU: ";
    text += String(getCpuFrequencyMhz());
    text += " MHz";

    text += "\nHeap: ";
    text += String(ESP.getFreeHeap());

    text += "\nFlash: ";
    text += String(ESP.getFlashChipSize());

    text += "\nSDK:\n";
    text += ESP.getSdkVersion();

    openTool(
        "SYSTEM INFO",
        text);
}

// ============================================================
// BRIGHTNESS TOOL
// ============================================================

void showBrightness()
{
    String text = "";

    text += "Brightness: ";

    text += String(screenBrightness);

    text += "\n\nUse ; and . later";
    text += "\nfor interactive control.";

    text += "\n\nCurrent value applied.";

    M5Cardputer.Display.setBrightness(
        screenBrightness);

    openTool(
        "BRIGHTNESS",
        text);
}

// ============================================================
// ABOUT
// ============================================================

void showAbout()
{
    String text = "";

    text += "Platform Pocket";
    text += "\nVersion 0.4";

    text += "\n\nESP32-S3 handheld";
    text += "\nplatform toolkit.";

    text += "\n\nBuilt while learning:";
    text += "\nC++";
    text += "\nNetworking";
    text += "\nEmbedded systems";

    openTool(
        "ABOUT",
        text);
}

// ============================================================
// EXECUTE SELECTED TOOL
// ============================================================
//
// This is the router.
//
// It asks:
//
// 1. Which MAIN section are we in?
// 2. Which SUBMENU item is selected?
// 3. Which function should run?
//
// This keeps loop() from becoming enormous.

void executeSelectedTool()
{
    int option =
        selectedSubItem[selectedMainItem];

    // ========================================================
    // NETWORK
    // ========================================================

    if (selectedMainItem == MAIN_NETWORK)
    {
        switch (option)
        {
        // Wi-Fi Scanner
        case 0:
            scanWifiNetworks();
            drawWifiResults();
            break;

        // Wi-Fi Info
        case 1:
            showWifiInfo();
            break;

        // Signal Monitor
        case 2:
            openTool(
                "SIGNAL MONITOR",
                "Live signal monitoring\n"
                "will be added next.\n\n"
                "This will graph RSSI\n"
                "over time.");
            break;

        // DNS Lookup
        case 3:
            openTool(
                "DNS LOOKUP",
                "DNS lookup needs an\n"
                "entered hostname and an\n"
                "active Wi-Fi connection.\n\n"
                "Keyboard input is next.");
            break;

        // Network Tools
        case 4:
            openTool(
                "NETWORK TOOLS",
                "Coming tools:\n\n"
                "- Ping helper\n"
                "- IP information\n"
                "- Gateway info\n"
                "- DNS information");
            break;
        }
    }

    // ========================================================
    // SECURITY
    // ========================================================

    else if (selectedMainItem == MAIN_SECURITY)
    {
        switch (option)
        {
        case 0:
            showSecurityDashboard();
            break;

        case 1:
            openTool(
                "WI-FI OBSERVATIONS",
                "Scanner observations:\n\n"
                "- Open networks\n"
                "- Hidden SSIDs\n"
                "- Duplicate SSIDs\n"
                "- Signal strength\n\n"
                "No attacker claims.");
            break;

        case 2:
            showDeviceInfo();
            break;

        case 3:
            openTool(
                "HASH TOOL",
                "Hash calculator planned.\n\n"
                "Future support:\n"
                "- SHA-256\n"
                "- Text input\n"
                "- File hashes");
            break;

        case 4:
            openTool(
                "PORT CHECK",
                "Defensive port checking\n"
                "will support hosts you\n"
                "own or administer.\n\n"
                "Target input comes next.");
            break;
        }
    }

    // ========================================================
    // DOCKER
    // ========================================================

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
            openTool(
                "REMOTE HOST",
                "Later Platform Pocket can\n"
                "talk to another computer\n"
                "running Docker.\n\n"
                "The ESP32 itself does not\n"
                "run Docker containers.");
            break;
        }
    }

    // ========================================================
    // NOTES
    // ========================================================

    else if (selectedMainItem == MAIN_NOTES)
    {
        switch (option)
        {
        case 0:
            openTool(
                "VIEW NOTES",
                "Notes will be stored on\n"
                "the SD card.\n\n"
                "Next step:\n"
                "filesystem support.");
            break;

        case 1:
            openTool(
                "NEW NOTE",
                "Keyboard text entry will\n"
                "allow you to create\n"
                "notes directly on the\n"
                "Cardputer.");
            break;

        case 2:
            openTool(
                "DELETE NOTE",
                "This page will list saved\n"
                "notes and allow one to\n"
                "be deleted.");
            break;

        case 3:
            openTool(
                "SD STORAGE",
                "SD card filesystem\n"
                "support will show:\n\n"
                "- Capacity\n"
                "- Free space\n"
                "- Note files");
            break;
        }
    }

    // ========================================================
    // QUICK TOOLS
    // ========================================================

    else if (selectedMainItem == MAIN_QUICK_TOOLS)
    {
        switch (option)
        {
        case 0:
            openTool(
                "IP TOOLS",
                "Planned:\n\n"
                "- IP parser\n"
                "- Private/public check\n"
                "- Gateway display\n"
                "- Local address");
            break;

        case 1:
            openTool(
                "SUBNET HELPER",
                "Planned examples:\n\n"
                "/24 = 255.255.255.0\n"
                "/16 = 255.255.0.0\n\n"
                "Calculator comes next.");
            break;

        case 2:
            openTool(
                "BASE CONVERTER",
                "Planned conversions:\n\n"
                "Decimal\n"
                "Binary\n"
                "Hexadecimal\n\n"
                "Keyboard entry required.");
            break;

        case 3:
            showPasswordGenerator();
            break;

        case 4:
            showSystemInfo();
            break;
        }
    }

    // ========================================================
    // SETTINGS
    // ========================================================

    else if (selectedMainItem == MAIN_SETTINGS)
    {
        switch (option)
        {
        case 0:
            showBrightness();
            break;

        case 1:
            openTool(
                "THEME",
                "Theme system planned:\n\n"
                "- Green terminal\n"
                "- Blue\n"
                "- Amber\n"
                "- High contrast");
            break;

        case 2:
            openTool(
                "WI-FI SETTINGS",
                "Later this page can save\n"
                "trusted Wi-Fi profiles\n"
                "for Platform Pocket.");
            break;

        case 3:
            openTool(
                "STORAGE",
                "Storage manager planned.\n\n"
                "Internal flash + SD\n"
                "information will appear\n"
                "here.");
            break;

        case 4:
            showAbout();
            break;
        }
    }
}

// ============================================================
// SETUP
// ============================================================
//
// Arduino programs begin with setup().
//
// setup() runs ONCE.
//
// Then loop() starts running repeatedly.

void setup()
{
    // Get standard M5Stack configuration.
    auto cfg = M5.config();

    // Initialize Cardputer hardware.
    //
    // Display
    // Keyboard
    // ESP32 peripherals
    //
    M5Cardputer.begin(cfg, true);

    // Landscape orientation.
    M5Cardputer.Display.setRotation(1);

    // Apply brightness setting.
    M5Cardputer.Display.setBrightness(
        screenBrightness);

    // Draw first screen.
    drawMainMenu();
}

// ============================================================
// LOOP
// ============================================================
//
// loop() runs forever.
//
// Conceptually:
//
// while (deviceIsOn)
// {
//     updateKeyboard();
//     inspectCurrentScreen();
//     reactToButtons();
// }
//
// ============================================================

void loop()
{
    // Update the Cardputer keyboard state.
    //
    // Without calling update(),
    // key presses may not be detected properly.

    M5Cardputer.update();

    // ========================================================
    // MAIN MENU
    // ========================================================

    if (currentScreen == SCREEN_MAIN)
    {
        // ----------------------------------------------------
        // UP
        // ----------------------------------------------------

        if (M5Cardputer.Keyboard.isKeyPressed(';'))
        {
            selectedMainItem--;

            // Wrap around.
            //
            // If we're above Network,
            // jump to Settings.

            if (selectedMainItem < 0)
            {
                selectedMainItem =
                    MAIN_COUNT - 1;
            }

            drawMainMenu();

            delay(150);
        }

        // ----------------------------------------------------
        // DOWN
        // ----------------------------------------------------

        if (M5Cardputer.Keyboard.isKeyPressed('.'))
        {
            selectedMainItem++;

            // If we pass Settings,
            // return to Network.

            if (selectedMainItem >= MAIN_COUNT)
            {
                selectedMainItem = 0;
            }

            drawMainMenu();

            delay(150);
        }

        // ----------------------------------------------------
        // ENTER
        // ----------------------------------------------------

        if (M5Cardputer.Keyboard.keysState().enter)
        {
            currentScreen =
                SCREEN_SECTION_MENU;

            drawSectionMenu();

            delay(200);
        }
    }

    // ========================================================
    // SECTION MENU
    // ========================================================

    else if (currentScreen == SCREEN_SECTION_MENU)
    {
        int count =
            getCurrentSubMenuCount();

        // Get selection for CURRENT section.
        //
        // The & means "reference".
        //
        // Instead of making a copy,
        // selection refers directly to the value
        // stored inside selectedSubItem[].
        //
        // So changing selection also changes
        // the original array value.

        int &selection =
            selectedSubItem[selectedMainItem];

        // ----------------------------------------------------
        // UP
        // ----------------------------------------------------

        if (M5Cardputer.Keyboard.isKeyPressed(';'))
        {
            selection--;

            if (selection < 0)
            {
                selection = count - 1;
            }

            drawSectionMenu();

            delay(150);
        }

        // ----------------------------------------------------
        // DOWN
        // ----------------------------------------------------

        if (M5Cardputer.Keyboard.isKeyPressed('.'))
        {
            selection++;

            if (selection >= count)
            {
                selection = 0;
            }

            drawSectionMenu();

            delay(150);
        }

        // ----------------------------------------------------
        // ENTER
        // ----------------------------------------------------

        if (M5Cardputer.Keyboard.keysState().enter)
        {
            executeSelectedTool();

            delay(200);
        }

        // ----------------------------------------------------
        // ESC
        // ----------------------------------------------------

        if (M5Cardputer.Keyboard.keysState().esc)
        {
            currentScreen = SCREEN_MAIN;

            drawMainMenu();

            delay(200);
        }
    }

    // ========================================================
    // GENERIC TOOL PAGE
    // ========================================================

    else if (currentScreen == SCREEN_TOOL)
    {
        if (M5Cardputer.Keyboard.keysState().esc)
        {
            currentScreen =
                SCREEN_SECTION_MENU;

            drawSectionMenu();

            delay(200);
        }
    }

    // ========================================================
    // WI-FI SCANNER RESULTS
    // ========================================================

    else if (currentScreen == SCREEN_WIFI_SCAN)
    {
        // ----------------------------------------------------
        // UP
        // ----------------------------------------------------

        if (M5Cardputer.Keyboard.isKeyPressed(';'))
        {
            if (wifiNetworkCount > 0)
            {
                selectedWifiNetwork--;

                // Wrap to last network.
                if (selectedWifiNetwork < 0)
                {
                    selectedWifiNetwork =
                        wifiNetworkCount - 1;
                }

                // Make sure selected item stays visible.
                if (
                    selectedWifiNetwork <
                    wifiScrollOffset)
                {
                    wifiScrollOffset =
                        selectedWifiNetwork;
                }

                // Handle wrap to bottom.
                if (
                    selectedWifiNetwork >=
                    wifiScrollOffset +
                        wifiRowsVisible)
                {
                    wifiScrollOffset =
                        selectedWifiNetwork -
                        wifiRowsVisible + 1;
                }

                drawWifiResults();
            }

            delay(150);
        }

        // ----------------------------------------------------
        // DOWN
        // ----------------------------------------------------

        if (M5Cardputer.Keyboard.isKeyPressed('.'))
        {
            if (wifiNetworkCount > 0)
            {
                selectedWifiNetwork++;

                if (
                    selectedWifiNetwork >=
                    wifiNetworkCount)
                {
                    selectedWifiNetwork = 0;
                }

                if (
                    selectedWifiNetwork >=
                    wifiScrollOffset +
                        wifiRowsVisible)
                {
                    wifiScrollOffset++;
                }

                if (
                    selectedWifiNetwork <
                    wifiScrollOffset)
                {
                    wifiScrollOffset = 0;
                }

                drawWifiResults();
            }

            delay(150);
        }

        // ----------------------------------------------------
        // ENTER
        // ----------------------------------------------------

        if (M5Cardputer.Keyboard.keysState().enter)
        {
            if (wifiNetworkCount > 0)
            {
                currentScreen =
                    SCREEN_WIFI_DETAILS;

                drawWifiDetails();
            }
            else
            {
                scanWifiNetworks();

                drawWifiResults();
            }

            delay(200);
        }

        // ----------------------------------------------------
        // ESC
        // ----------------------------------------------------

        if (M5Cardputer.Keyboard.keysState().esc)
        {
            currentScreen =
                SCREEN_SECTION_MENU;

            drawSectionMenu();

            delay(200);
        }
    }

    // ========================================================
    // WI-FI NETWORK DETAILS
    // ========================================================

    else if (currentScreen == SCREEN_WIFI_DETAILS)
    {
        if (M5Cardputer.Keyboard.keysState().esc)
        {
            currentScreen =
                SCREEN_WIFI_SCAN;

            drawWifiResults();

            delay(200);
        }
    }
}