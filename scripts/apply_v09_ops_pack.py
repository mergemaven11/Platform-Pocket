from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
MAIN = ROOT / "src" / "main.cpp"
WORKSTATION = ROOT / "src" / "workstation.cpp"
README = ROOT / "README.md"


def replace_once(text: str, old: str, new: str, label: str) -> str:
    if new in text:
        return text
    if old not in text:
        raise RuntimeError(f"Could not find expected block for {label}")
    return text.replace(old, new, 1)


def collapse_repeats(text: str, block: str, label: str) -> str:
    if block not in text:
        raise RuntimeError(f"Could not find expected repeated block for {label}")
    while block + block in text:
        text = text.replace(block + block, block)
    return text


main = MAIN.read_text()
main = main.replace('static const char *APP_VERSION = "0.8";', 'static const char *APP_VERSION = "0.9";', 1)

help_old = '''        terminalPush("wifi scan ip net dns HOST");
        terminalPush("port HOST PORT sha256 TEXT");
        terminalPush("cidr N base N diag sysinfo");
        terminalPush("sd workspace note TEXT notes");
        terminalPush("files edit snapshot diff runbooks");
        terminalPush("runbook NAME incident ...");
        terminalPush("troubleshoot TEXT uptime version");'''
help_new = '''        terminalPush("net: wifi scan ip dns HOST");
        terminalPush("svc: port/probe HOST PORT");
        terminalPush("ops: health diag k8s docker git");
        terminalPush("util: sha256 cidr base uptime");
        terminalPush("sd: note notes files snapshot diff");
        terminalPush("runbooks incident troubleshoot");'''
main = replace_once(main, help_old, help_new, "terminal help")

workstation_block = '''    else if (lower == "files")
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
main = collapse_repeats(main, workstation_block, "workstation command handlers")

editor_block = '''    if (currentScreen == SCREEN_EDITOR)
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
                editorBuffer += '\\n';
            PocketWorkstation::saveEditorNote(editorBuffer);
        }
        drawEditor();
        return;
    }

'''
main = collapse_repeats(main, editor_block, "editor input handler")

ops_handlers = '''    else if (lower == "health")
    {
        terminalPush(String("wifi: ") + (WiFi.status() == WL_CONNECTED ? "connected" : "offline"));
        if (WiFi.status() == WL_CONNECTED)
            terminalPush(String("rssi: ") + WiFi.RSSI() + " dBm " + getSignalLabel(WiFi.RSSI()));
        terminalPush(String("heap: ") + ESP.getFreeHeap() + " bytes");
        terminalPush(String("minheap: ") + ESP.getMinFreeHeap() + " bytes");
        terminalPush(String("sd: ") + (PocketStorage::ready() ? "ready" : "offline"));
        terminalPush(String("uptime: ") + millis() / 1000UL + " sec");
    }
    else if (lower.startsWith("probe "))
    {
        if (WiFi.status() != WL_CONNECTED)
        {
            terminalPush("! connect to Wi-Fi first");
        }
        else
        {
            int split = command.indexOf(' ', 6);
            if (split < 0)
            {
                terminalPush("usage: probe HOST PORT");
            }
            else
            {
                String host = command.substring(6, split);
                int port = command.substring(split + 1).toInt();
                if (host.length() == 0 || port < 1 || port > 65535)
                {
                    terminalPush("usage: probe HOST PORT");
                }
                else
                {
                    IPAddress resolved;
                    unsigned long dnsStart = millis();
                    int dnsOk = WiFi.hostByName(host.c_str(), resolved);
                    unsigned long dnsMs = millis() - dnsStart;
                    if (dnsOk != 1)
                    {
                        terminalPush("! DNS lookup failed");
                    }
                    else
                    {
                        terminalPush(String("dns ") + dnsMs + "ms " + resolved.toString());
                        WiFiClient client;
                        unsigned long tcpStart = millis();
                        bool reachable = client.connect(host.c_str(), static_cast<uint16_t>(port), 1500);
                        unsigned long tcpMs = millis() - tcpStart;
                        terminalPush(String("tcp ") + tcpMs + "ms " + (reachable ? "OPEN" : "NO REPLY"));
                        client.stop();
                    }
                }
            }
        }
    }
    else if (lower == "k8s" || lower == "kubectl")
    {
        terminalPush("kubectl get pods -A");
        terminalPush("kubectl describe pod NAME");
        terminalPush("kubectl logs -f POD");
        terminalPush("kubectl get events --sort-by=.metadata.creationTimestamp");
    }
    else if (lower == "git")
    {
        terminalPush("git status / log --oneline");
        terminalPush("git diff / diff --staged");
        terminalPush("git fetch / pull --ff-only");
        terminalPush("git switch -c BRANCH");
    }
'''
marker = '    else if (lower == "sysinfo" || lower == "free")\n'
if '    else if (lower == "health")\n' not in main:
    if marker not in main:
        raise RuntimeError("Could not find sysinfo insertion point")
    main = main.replace(marker, ops_handlers + marker, 1)

MAIN.write_text(main)

workstation = WORKSTATION.read_text()
seed_old = '''        {"memory.md", "# Device instability\\n1. Run `diag`.\\n2. Compare free heap and minimum heap.\\n3. Check uptime for unexpected reset evidence.\\n4. Save `snapshot` before reproducing.\\n"}};'''
seed_new = '''        {"memory.md", "# Device instability\\n1. Run `diag`.\\n2. Compare free heap and minimum heap.\\n3. Check uptime for unexpected reset evidence.\\n4. Save `snapshot` before reproducing.\\n"},
        {"kubernetes.md", "# Kubernetes workload issue\\n1. Check cluster context from an authorized admin host.\\n2. Run `kubectl get pods -A`.\\n3. Describe the affected pod and inspect recent events.\\n4. Review logs before restarting or changing workloads.\\n5. Record findings in an incident note.\\n"},
        {"container.md", "# Container service issue\\n1. Confirm host/network reachability.\\n2. Check container state and recent logs.\\n3. Validate the published service port with `probe HOST PORT`.\\n4. Capture a snapshot before making changes.\\n"},
        {"incident.md", "# Incident quick start\\n1. `incident new TITLE`\\n2. `health` and `snapshot`\\n3. Add observations with `incident add TEXT`\\n4. Use `diff` after another snapshot.\\n5. Preserve evidence before remediation.\\n"}};'''
workstation = replace_once(workstation, seed_old, seed_new, "runbook seeds")

summary_old = '    return "RUNBOOKS\\ndns\\nwifi\\nservice\\nmemory\\n\\nrunbook NAME";'
summary_new = '    return "RUNBOOKS\\ndns wifi service memory\\nkubernetes container incident\\n\\nrunbook NAME";'
workstation = replace_once(workstation, summary_old, summary_new, "runbook summary")

trouble_marker = '    if (s.indexOf("reset") >= 0 || s.indexOf("memory") >= 0 || s.indexOf("crash") >= 0)\n        return "Check device health.\\n1 diag\\n2 min heap\\n3 uptime\\n4 snapshot\\nRunbook: memory";\n'
trouble_insert = trouble_marker + '''    if (s.indexOf("k8s") >= 0 || s.indexOf("kubernetes") >= 0 || s.indexOf("pod") >= 0)
        return "Check workload path.\\n1 k8s\\n2 pod describe/events\\n3 logs\\n4 incident note\\nRunbook: kubernetes";
    if (s.indexOf("docker") >= 0 || s.indexOf("container") >= 0)
        return "Check container path.\\n1 net\\n2 probe HOST PORT\\n3 host logs/state\\n4 snapshot\\nRunbook: container";
'''
if 'Runbook: kubernetes' not in workstation:
    workstation = replace_once(workstation, trouble_marker, trouble_insert, "troubleshooting branches")

WORKSTATION.write_text(workstation)

readme = README.read_text()
readme = readme.replace('The v0.5 refresh focuses on making the device feel like a real product instead of a firmware demo:', 'The v0.9 production-ops refresh builds on the handheld workstation with faster field diagnostics and practical platform-engineering references:', 1)
readme = readme.replace('## ✨ v0.5 Highlights', '## ✨ v0.9 Highlights', 1)
if '- Production health summary with `health`' not in readme:
    anchor = '- Input handling now uses Cardputer keyboard change events to prevent repeated held-key input\n'
    addition = anchor + '- Production health summary with `health`\n- Timed DNS + TCP service probing with `probe HOST PORT`\n- Kubernetes and Git pocket references in the terminal\n- Additional offline Kubernetes, container, and incident-response runbooks\n- v0.8 duplicate command/editor integration blocks cleaned up\n'
    readme = replace_once(readme, anchor, addition, "v0.9 highlights")

commands_anchor = '| `docker` | Quick Docker / Compose reference |\n'
commands_add = commands_anchor + '| `health` | Show Wi-Fi, RSSI, heap, SD, and uptime health |\n| `probe HOST PORT` | Resolve a host and time an authorized TCP service check |\n| `k8s` / `kubectl` | Show Kubernetes troubleshooting command references |\n| `git` | Show common Git investigation/workflow commands |\n'
readme = replace_once(readme, commands_anchor, commands_add, "command table")
readme = readme.replace('**Platform Pocket v0.5 is an active development build.**', '**Platform Pocket v0.9 is an active development build.**', 1)
README.write_text(readme)

print("Platform Pocket v0.9 Production Ops Pack applied.")
