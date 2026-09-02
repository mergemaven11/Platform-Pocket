#include "storage.h"

#include <M5Cardputer.h>
#include <SD.h>
#include <SPI.h>
#include <WiFi.h>
#include <esp_system.h>

namespace
{
constexpr int SD_SPI_SCK_PIN = 40;
constexpr int SD_SPI_MISO_PIN = 39;
constexpr int SD_SPI_MOSI_PIN = 14;
constexpr int SD_SPI_CS_PIN = 12;
constexpr uint32_t SD_SPI_FREQUENCY = 25000000;
const char *WORKSPACE_ROOT = "/platform-pocket";
const char *NOTE_INBOX = "/platform-pocket/notes/inbox.md";
bool storageReady = false;

const char *workspaceDirectories[] = {
    "/platform-pocket/notes",
    "/platform-pocket/incidents",
    "/platform-pocket/diagnostics",
    "/platform-pocket/logs",
    "/platform-pocket/runbooks",
    "/platform-pocket/snippets",
    "/platform-pocket/profiles",
    "/platform-pocket/exports",
    "/platform-pocket/config"};

/** @brief Ensure one workspace directory exists. */
bool ensureDirectory(const char *path)
{
    return SD.exists(path) || SD.mkdir(path);
}

/** @brief Ensure the full Platform Pocket workspace tree exists. */
bool ensureWorkspace()
{
    if (!ensureDirectory(WORKSPACE_ROOT))
        return false;
    for (const char *path : workspaceDirectories)
        if (!ensureDirectory(path))
            return false;
    return true;
}

/** @brief Return a compact label for the mounted card type. */
const char *cardTypeLabel()
{
    switch (SD.cardType())
    {
    case CARD_MMC:
        return "MMC";
    case CARD_SD:
        return "SDSC";
    case CARD_SDHC:
        return "SDHC/SDXC";
    default:
        return "UNKNOWN";
    }
}

/** @brief Format bytes as gigabytes for the handheld display. */
String gigabytes(uint64_t bytes)
{
    return String(static_cast<double>(bytes) / (1024.0 * 1024.0 * 1024.0), 1) + " GB";
}

/** @brief Find the next unused diagnostic snapshot filename. */
String nextSnapshotPath()
{
    for (int index = 1; index <= 9999; ++index)
    {
        char path[64];
        snprintf(path, sizeof(path), "/platform-pocket/diagnostics/snapshot-%04d.txt", index);
        if (!SD.exists(path))
            return String(path);
    }
    return "";
}
} // namespace

namespace PocketStorage
{
bool begin()
{
    storageReady = false;
    SPI.begin(SD_SPI_SCK_PIN, SD_SPI_MISO_PIN, SD_SPI_MOSI_PIN, SD_SPI_CS_PIN);
    if (!SD.begin(SD_SPI_CS_PIN, SPI, SD_SPI_FREQUENCY))
        return false;
    if (SD.cardType() == CARD_NONE || !ensureWorkspace())
        return false;
    storageReady = true;
    return true;
}

bool ready()
{
    return storageReady;
}

String statusText()
{
    if (!storageReady)
        return "SD: offline\nInsert/mount card to enable\npersistent workspace.";

    uint64_t total = SD.totalBytes();
    uint64_t used = SD.usedBytes();
    uint64_t free = total >= used ? total - used : 0;
    String text = String("Card: ") + cardTypeLabel();
    text += "\nTotal: " + gigabytes(total);
    text += "\nUsed: " + gigabytes(used);
    text += "\nFree: " + gigabytes(free);
    text += "\nRoot: /platform-pocket";
    return text;
}

bool appendQuickNote(const String &text)
{
    if (!storageReady || text.length() == 0)
        return false;
    File file = SD.open(NOTE_INBOX, FILE_APPEND);
    if (!file)
        return false;
    file.print("- [uptime ");
    file.print(millis() / 1000UL);
    file.print("s] ");
    file.println(text);
    file.close();
    return true;
}

String noteSummary()
{
    if (!storageReady)
        return "SD workspace offline.\nQuick notes require a mounted card.";
    File file = SD.open(NOTE_INBOX, FILE_READ);
    if (!file)
        return "Inbox is empty.\n\nTerminal:\nnote TEXT";
    size_t bytes = file.size();
    file.close();
    return "Inbox: notes/inbox.md\nSize: " + String(bytes) + " bytes\n\nTerminal:\nnote TEXT\nnotes";
}

String saveDiagnosticSnapshot()
{
    if (!storageReady)
        return "! SD workspace offline";
    String path = nextSnapshotPath();
    if (path.length() == 0)
        return "! snapshot slots exhausted";
    File file = SD.open(path.c_str(), FILE_WRITE);
    if (!file)
        return "! snapshot write failed";

    file.println("Platform Pocket diagnostic snapshot");
    file.println("==================================");
    file.print("uptime_seconds="); file.println(millis() / 1000UL);
    file.print("free_heap="); file.println(ESP.getFreeHeap());
    file.print("min_free_heap="); file.println(ESP.getMinFreeHeap());
    file.print("cpu_mhz="); file.println(getCpuFrequencyMhz());
    file.print("wifi_status="); file.println(WiFi.status() == WL_CONNECTED ? "connected" : "offline");
    if (WiFi.status() == WL_CONNECTED)
    {
        file.print("ssid="); file.println(WiFi.SSID());
        file.print("rssi_dbm="); file.println(WiFi.RSSI());
        file.print("channel="); file.println(WiFi.channel());
        file.print("ip="); file.println(WiFi.localIP().toString());
        file.print("gateway="); file.println(WiFi.gatewayIP().toString());
        file.print("subnet="); file.println(WiFi.subnetMask().toString());
        file.print("dns="); file.println(WiFi.dnsIP().toString());
    }
    file.close();
    return path;
}

String workspaceSummary()
{
    if (!storageReady)
        return "Workspace offline.\nMount SD card first.";
    return "/platform-pocket/\nnotes incidents diagnostics\nlogs runbooks snippets\nprofiles exports config";
}
} // namespace PocketStorage
