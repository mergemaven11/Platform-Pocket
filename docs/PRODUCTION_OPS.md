# Platform Pocket Production Ops Pack

Platform Pocket v0.9 adds a compact production-support layer for field diagnostics and platform-engineering work.

## Live health check

From the local terminal:

```text
health
```

The command summarizes:

- Wi-Fi connection state
- RSSI and signal quality when connected
- current free heap
- minimum observed free heap
- SD workspace readiness
- device uptime

Use this before and after a troubleshooting action to quickly spot device or connectivity degradation.

## Timed service probe

```text
probe HOST PORT
```

Example:

```text
probe example.com 443
```

The probe performs two bounded checks:

1. DNS resolution and elapsed lookup time
2. TCP connection attempt and elapsed connection time

This is intended for systems you own or are authorized to administer. It is a focused reachability diagnostic, not a port scanner.

## Kubernetes pocket reference

```text
k8s
```

or:

```text
kubectl
```

The terminal displays a small set of high-value investigation commands for pod state, descriptions, logs, and events. Platform Pocket does not execute kubectl locally; these are references for use on an authorized Linux/admin host.

## Git pocket reference

```text
git
```

The terminal displays common investigation and safe workflow commands for status, logs, diffs, fetching, fast-forward pulls, and branch creation.

## Offline runbooks

The SD-backed runbook library now includes:

- `dns`
- `wifi`
- `service`
- `memory`
- `kubernetes`
- `container`
- `incident`

Use:

```text
runbooks
runbook kubernetes
runbook container
runbook incident
```

The troubleshooting helper also recognizes Kubernetes/pod and Docker/container symptoms.

## Incident workflow

A useful field sequence is:

```text
incident new api-outage
health
snapshot
probe api.internal 443
incident add API probe returned no reply
snapshot
diff
```

This keeps observations and before/after state together on the SD workspace.

## v0.8 cleanup included

The v0.9 integration also removes repeated workstation command-handler and Markdown-editor input blocks that were accidentally duplicated during the v0.8 generated integration. Behavior is preserved while the main event loop and terminal parser become easier to maintain.
