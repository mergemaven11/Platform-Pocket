#include "workstation.h"

#include <SD.h>
#include "storage.h"

namespace
{
const char *EDITOR_NOTE = "/platform-pocket/notes/editor.md";
const char *INCIDENT_INDEX = "/platform-pocket/incidents/current.txt";

/** @brief Write a text file, replacing existing content. */
bool writeTextFile(const String &path, const String &text)
{
    File file = SD.open(path.c_str(), FILE_WRITE);
    if (!file)
        return false;
    file.print(text);
    file.close();
    return true;
}

/** @brief Read a text file with a bounded in-memory payload. */
String readTextFile(const String &path, size_t limit = 1400)
{
    File file = SD.open(path.c_str(), FILE_READ);
    if (!file)
        return "";
    String out;
    out.reserve(limit);
    while (file.available() && out.length() < limit)
        out += static_cast<char>(file.read());
    file.close();
    return out;
}

/** @brief Return a sanitized short file token. */
String slugify(String value)
{
    value.toLowerCase();
    String out;
    for (char c : value)
    {
        if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9'))
            out += c;
        else if ((c == ' ' || c == '-' || c == '_') && !out.endsWith("-"))
            out += '-';
    }
    if (out.length() == 0)
        out = "incident";
    return out.substring(0, min(static_cast<int>(out.length()), 24));
}

/** @brief Create bundled runbooks if they do not already exist. */
void ensureRunbooks()
{
    if (!PocketStorage::ready())
        return;
    struct RunbookSeed { const char *name; const char *body; };
    const RunbookSeed seeds[] = {
        {"dns.md", "# DNS failure\n1. Check IP/gateway with `net`.\n2. Resolve a known host with `dns HOST`.\n3. If IP works but names fail, inspect DNS server.\n4. Capture `snapshot` before changes.\n"},
        {"wifi.md", "# Wi-Fi weak/intermittent\n1. Check `wifi` and Signal Monitor.\n2. Compare RSSI while moving.\n3. Scan nearby channels.\n4. Capture a snapshot at good and bad locations.\n"},
        {"service.md", "# Service unreachable\n1. Confirm network with `net`.\n2. Resolve host with `dns HOST`.\n3. Test one authorized service using `port HOST PORT`.\n4. Check remote service/firewall from an authorized admin host.\n"},
        {"memory.md", "# Device instability\n1. Run `diag`.\n2. Compare free heap and minimum heap.\n3. Check uptime for unexpected reset evidence.\n4. Save `snapshot` before reproducing.\n"}};
    for (const auto &seed : seeds)
    {
        String path = String("/platform-pocket/runbooks/") + seed.name;
        if (!SD.exists(path.c_str()))
            writeTextFile(path, seed.body);
    }
}

/** @brief Find the newest two snapshot paths by filename ordering. */
void newestSnapshots(String &older, String &newer)
{
    older = "";
    newer = "";
    File dir = SD.open("/platform-pocket/diagnostics");
    if (!dir || !dir.isDirectory())
        return;
    File entry = dir.openNextFile();
    while (entry)
    {
        if (!entry.isDirectory())
        {
            String name = entry.name();
            if (name.indexOf("snapshot-") >= 0)
            {
                String path = name.startsWith("/") ? name : String("/platform-pocket/diagnostics/") + name;
                if (path > newer)
                {
                    older = newer;
                    newer = path;
                }
                else if (path > older)
                    older = path;
            }
        }
        entry.close();
        entry = dir.openNextFile();
    }
    dir.close();
}

/** @brief Extract one key=value line from a snapshot. */
String snapshotValue(const String &text, const String &key)
{
    String needle = key + "=";
    int start = text.indexOf(needle);
    if (start < 0)
        return "-";
    start += needle.length();
    int end = text.indexOf('\n', start);
    if (end < 0)
        end = text.length();
    return text.substring(start, end);
}
} // namespace

namespace PocketWorkstation
{
/** @brief Initialize offline workstation content on mounted storage. */
void begin()
{
    ensureRunbooks();
}

/** @brief List workspace files without loading large file contents. */
String fileManagerSummary()
{
    if (!PocketStorage::ready())
        return "SD workspace offline.";
    const char *dirs[] = {"notes", "incidents", "diagnostics", "runbooks"};
    String out = "FILES\n";
    for (const char *dirName : dirs)
    {
        String path = String("/platform-pocket/") + dirName;
        File dir = SD.open(path.c_str());
        int count = 0;
        if (dir && dir.isDirectory())
        {
            File entry = dir.openNextFile();
            while (entry)
            {
                if (!entry.isDirectory())
                    ++count;
                entry.close();
                entry = dir.openNextFile();
            }
            dir.close();
        }
        out += String(dirName) + ": " + count + "\n";
    }
    out += "Terminal: files / runbooks";
    return out;
}

/** @brief Save the bounded full-screen Markdown editor buffer. */
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

/** @brief Create and select a Markdown incident notebook. */
String createIncident(const String &title)
{
    if (!PocketStorage::ready())
        return "! SD offline";
    String path = String("/platform-pocket/incidents/") + slugify(title) + ".md";
    String body = "# " + title + "\n\n- created at uptime " + String(millis() / 1000UL) + "s\n";
    if (!writeTextFile(path, body) || !writeTextFile(INCIDENT_INDEX, path))
        return "! incident create failed";
    return path;
}

/** @brief Append an observation to the selected incident. */
bool appendIncident(const String &text)
{
    if (!PocketStorage::ready())
        return false;
    String path = readTextFile(INCIDENT_INDEX, 128);
    path.trim();
    if (path.length() == 0)
        return false;
    File file = SD.open(path.c_str(), FILE_APPEND);
    if (!file)
        return false;
    file.print("- [uptime ");
    file.print(millis() / 1000UL);
    file.print("s] ");
    file.println(text);
    file.close();
    return true;
}

/** @brief Summarize the active incident notebook. */
String incidentSummary()
{
    if (!PocketStorage::ready())
        return "SD workspace offline.";
    String path = readTextFile(INCIDENT_INDEX, 128);
    path.trim();
    if (path.length() == 0)
        return "No active incident.\nincident new TITLE";
    return "Active:\n" + path + "\nincident add TEXT\nsnapshot";
}

/** @brief Compare selected network and runtime fields across two snapshots. */
String compareSnapshots()
{
    if (!PocketStorage::ready())
        return "SD workspace offline.";
    String older, newer;
    newestSnapshots(older, newer);
    if (older.length() == 0 || newer.length() == 0)
        return "Need at least 2 snapshots.";
    String a = readTextFile(older);
    String b = readTextFile(newer);
    const char *keys[] = {"wifi_status", "ssid", "rssi_dbm", "ip", "gateway", "dns", "free_heap"};
    String out = "SNAPSHOT DIFF\n";
    int changes = 0;
    for (const char *key : keys)
    {
        String av = snapshotValue(a, key);
        String bv = snapshotValue(b, key);
        if (av != bv)
        {
            out += String(key) + ": " + av + " -> " + bv + "\n";
            ++changes;
            if (out.length() > 900)
                break;
        }
    }
    if (changes == 0)
        out += "No tracked differences.";
    return out;
}

/** @brief List the bundled offline troubleshooting runbooks. */
String runbookSummary()
{
    if (!PocketStorage::ready())
        return "SD workspace offline.";
    ensureRunbooks();
    return "RUNBOOKS\ndns\nwifi\nservice\nmemory\n\nrunbook NAME";
}

/** @brief Read one bundled Markdown runbook. */
String readRunbook(const String &name)
{
    if (!PocketStorage::ready())
        return "SD workspace offline.";
    String clean = slugify(name);
    String path = String("/platform-pocket/runbooks/") + clean + ".md";
    String body = readTextFile(path, 900);
    return body.length() ? body : "Runbook not found.";
}

/** @brief Apply a small offline symptom-to-checklist troubleshooting tree. */
String troubleshoot(const String &symptom)
{
    String s = symptom;
    s.toLowerCase();
    if (s.indexOf("dns") >= 0 || s.indexOf("hostname") >= 0 || s.indexOf("name") >= 0)
        return "Likely DNS path.\n1 net\n2 dns HOST\n3 compare DNS/gateway\n4 snapshot\nRunbook: dns";
    if (s.indexOf("wifi") >= 0 || s.indexOf("signal") >= 0 || s.indexOf("slow") >= 0)
        return "Check wireless path.\n1 wifi\n2 signal monitor\n3 scan channels\n4 snapshot twice\nRunbook: wifi";
    if (s.indexOf("port") >= 0 || s.indexOf("service") >= 0 || s.indexOf("connect") >= 0)
        return "Check service path.\n1 net\n2 dns HOST\n3 port HOST PORT\n4 remote service/firewall\nRunbook: service";
    if (s.indexOf("reset") >= 0 || s.indexOf("memory") >= 0 || s.indexOf("crash") >= 0)
        return "Check device health.\n1 diag\n2 min heap\n3 uptime\n4 snapshot\nRunbook: memory";
    return "Start broad:\n1 diag\n2 net\n3 snapshot\nThen: troubleshoot dns|wifi|service|memory";
}
} // namespace PocketWorkstation
