# Changelog

Notable Platform Pocket changes are recorded here.

## v0.9 - release candidate

### Added

- Production `health` command for Wi-Fi, heap, SD, and uptime status.
- Bounded `probe HOST PORT` DNS/TCP service diagnostic.
- Kubernetes and Git terminal reference commands.
- Kubernetes, container, and incident-response offline runbooks.
- SD-backed multi-file text editor for `.md` and `.txt` files.
- 8 KB editor working-document limit.
- `edit NAME`, `new NAME`, `saveas NAME`, and `textfiles` workflows.
- Production readiness and physical Cardputer ADV smoke-test checklist.
- CI firmware artifacts for physical smoke testing.
- Tag-driven GitHub Release workflow with SHA-256 checksums.

### Changed

- PlatformIO, ESP32 platform, native test platform, and M5Stack dependencies are pinned for reproducible builds.
- CI uses current GitHub Actions releases and pinned PlatformIO.
- Firmware binary size is gated at 2,500,000 bytes.
- Native tests cover all Wi-Fi signal-quality threshold boundaries.
- Upload port is no longer hard-coded to a developer-specific serial port.

### Fixed

- Removed duplicated v0.8 workstation command-handler integration blocks.
- Removed duplicated editor input-handler integration blocks.
- Editor now preserves a dirty in-memory document when an SD save fails instead of leaving the editor and silently treating the save as successful.

### Release criteria

v0.9 should be tagged only after CI is green and the generated firmware artifact passes the physical smoke test documented in `docs/PRODUCTION.md`.
