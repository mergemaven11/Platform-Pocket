"""Ensure the generated Platform Pocket v0.6 field-tools source stays buildable."""

from pathlib import Path

path = Path("src/main.cpp")
source = path.read_text(encoding="utf-8")

if 'static const char *APP_VERSION = "0.6";' not in source:
    raise SystemExit("Platform Pocket v0.6 field-tools source has not been generated")

required = [
    "String formatIpv4(uint32_t value)",
    "String toBinary(uint32_t value)",
    "String sha256Text(const String &value)",
    "void showSignalMonitor()",
    "void showNetworkTools()",
    "void showStorageInfo()",
]
missing = [token for token in required if token not in source]
if missing:
    raise SystemExit(f"Missing generated field-tool definitions: {missing}")

anchor = 'String previousTerminalCommand = "";\n'
declarations = '''String previousTerminalCommand = "";\n\n// Forward declarations for terminal field-tool helpers.\nString formatIpv4(uint32_t value);\nString toBinary(uint32_t value);\nString sha256Text(const String &value);\nvoid showSignalMonitor();\nvoid showNetworkTools();\nvoid showHashToolHelp();\nvoid showPortCheckHelp();\nvoid showIpTools();\nvoid showSubnetHelper();\nvoid showBaseConverter();\nvoid showStorageInfo();\n'''

if "// Forward declarations for terminal field-tool helpers." not in source:
    if anchor not in source:
        raise SystemExit("Could not locate terminal state anchor")
    source = source.replace(anchor, declarations, 1)
    path.write_text(source, encoding="utf-8")
    print("Added field-tool forward declarations.")
else:
    print("Field-tool forward declarations already present.")
