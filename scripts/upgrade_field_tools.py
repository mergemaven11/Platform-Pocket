"""Upgrade Platform Pocket's handheld shell with real diagnostic field tools."""

from pathlib import Path

PATH = Path("src/main.cpp")
source = PATH.read_text(encoding="utf-8")


def replace_once(old: str, new: str) -> None:
    """Replace one required source fragment or fail loudly."""
    global source
    if old not in source:
        raise SystemExit(f"Expected source fragment not found:\n{old[:160]}")
    source = source.replace(old, new, 1)


replace_once(
    "#include <WiFi.h>\n#include <esp_system.h>",
    "#include <WiFi.h>\n#include <WiFiClient.h>\n#include <esp_system.h>\n#include <mbedtls/sha256.h>",
)
replace_once(
    '// PLATFORM POCKET v0.5\n// A compact handheld platform / network toolkit for Cardputer ADV.',
    '// PLATFORM POCKET v0.6\n// A compact handheld platform / network toolkit for Cardputer ADV.',
)
replace_once('static const char *APP_VERSION = "0.5";', 'static const char *APP_VERSION = "0.6";')
replace_once(
    'String lastTerminalCommand = "";',
    'String lastTerminalCommand = "";\nString previousTerminalCommand = "";',
)
replace_once(
    "    wifiNetworkCount = WiFi.scanNetworks(false, true);\n    selectedWifiNetwork = 0;",
    "    wifiNetworkCount = WiFi.scanNetworks(false, true);\n    if (wifiNetworkCount < 0)\n        wifiNetworkCount = 0;\n    selectedWifiNetwork = 0;",
)
replace_once(
    "    lastTerminalCommand = command;\n    terminalPush(String(\"> \") + command);",
    "    previousTerminalCommand = lastTerminalCommand;\n    lastTerminalCommand = command;\n    terminalPush(String(\"> \") + command);",
)
replace_once(
    '        terminalPush("help clear wifi scan ip");\n        terminalPush("sysinfo uptime docker history");\n        terminalPush("version ssh");',
    '        terminalPush("wifi scan ip net dns HOST");\n        terminalPush("port HOST PORT sha256 TEXT");\n        terminalPush("cidr N base N diag sysinfo");\n        terminalPush("uptime docker history version");',
)
replace_once(
    '        terminalPush(lastTerminalCommand.length() ? String("last: ") + lastTerminalCommand : "history empty");',
    '        terminalPush(previousTerminalCommand.length() ? String("last: ") + previousTerminalCommand : "history empty");',
)

insert_before = "/**\n * @brief Document showWifiInfo.\n */\nvoid showWifiInfo()"
if insert_before not in source:
    raise SystemExit("Could not locate field-tools insertion point")

helpers = r'''
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

'''
source = source.replace(insert_before, helpers + insert_before, 1)

needle = '''    else if (lower == "ip")
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
'''
replacement = needle + r'''    else if (lower == "net")
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
'''
replace_once(needle, replacement)

replace_once(
    '            openTool("SIGNAL MONITOR", "Live RSSI graph is queued.\\n\\nWi-Fi scanner already shows\\nper-network signal strength.");',
    '            showSignalMonitor();',
)
replace_once(
    '            openTool("DNS LOOKUP", "Hostname input is queued.\\n\\nRequires an active Wi-Fi\\nconnection to resolve DNS.");',
    '            openTool("DNS LOOKUP", "DNS resolver is live.\\n\\nTerminal example:\\ndns example.com\\n\\nRequires connected Wi-Fi.");',
)
replace_once(
    '            openTool("NETWORK TOOLS", "Available in Terminal now:\\n\\nscan\\nip\\nwifi\\nsysinfo\\nuptime");',
    '            showNetworkTools();',
)
replace_once(
    '            openTool("HASH TOOL", "SHA-256 text/file hashing\\nis planned for a later build.");',
    '            showHashToolHelp();',
)
replace_once(
    '            openTool("PORT CHECK", "Defensive port checks for\\nhosts you own/administer\\nare planned.");',
    '            showPortCheckHelp();',
)
replace_once(
    '            openTool("IP TOOLS", "Terminal commands now include:\\n\\nip\\nwifi\\nscan\\n\\nMore parsers are queued.");',
    '            showIpTools();',
)
replace_once(
    '            openTool("SUBNET HELPER", "/24 = 255.255.255.0\\n/16 = 255.255.0.0\\n/8  = 255.0.0.0\\n\\nCalculator is queued.");',
    '            showSubnetHelper();',
)
replace_once(
    '            openTool("BASE CONVERTER", "Decimal / binary / hex\\nconversion is queued.");',
    '            showBaseConverter();',
)
replace_once(
    '            openTool("STORAGE", "Internal flash + SD status\\nwill live here.");',
    '            showStorageInfo();',
)
replace_once(
    '    text += "\\n\\nUI + terminal refresh.";',
    '    text += "\\n\\nField tools + diagnostics.";',
)

PATH.write_text(source, encoding="utf-8")
print("Platform Pocket v0.6 field-tools upgrade applied.")
