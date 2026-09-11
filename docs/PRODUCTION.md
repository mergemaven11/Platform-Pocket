# Platform Pocket production readiness

This document defines what "production" means for Platform Pocket on the M5Stack Cardputer ADV.

A green compile is necessary, but it is not enough. A production release must be reproducible, pass automated tests, preserve operator data when storage is available, survive normal offline use, and complete a short hardware smoke test before a release tag is pushed.

## Automated release gates

Every pull request to `main` must pass the repository CI workflow.

The production gate currently requires:

- Cardputer ADV firmware compiles in PlatformIO release mode.
- Native unit tests pass.
- `firmware.bin`, `bootloader.bin`, and `partitions.bin` are produced.
- `firmware.bin` remains below the 2,500,000-byte release budget.
- A smoke-test firmware artifact is uploaded from CI for physical-device validation.
- PlatformIO, the ESP32 platform, and M5Stack libraries are pinned to known-good versions.

The upload serial port is intentionally not committed. PlatformIO should auto-detect the device, or an operator can override `upload_port` locally.

## Physical Cardputer ADV smoke test

Run this checklist on the CI firmware artifact before tagging a release.

### Boot and navigation

- Cold boot from battery power.
- Cold boot over USB power.
- Confirm the main menu renders without corruption.
- Enter and exit each top-level section.
- Leave the device idle for at least 30 minutes and confirm it remains responsive.

### Storage

Test once with a healthy microSD card inserted and once with no card installed.

With microSD inserted:

1. Open `Notes -> Markdown Editor`.
2. Create text, save it, leave the editor, and reopen it.
3. Create a named `.md` file with `new release-test.md`.
4. Confirm `textfiles` lists it.
5. Power-cycle the Cardputer and confirm the text is still present.
6. Run `snapshot`, then confirm a diagnostic snapshot is written.

Without microSD inserted:

- The device must still boot.
- Menus and non-storage tools must remain usable.
- Storage-dependent actions must fail cleanly instead of freezing or rebooting.

### Network tools

On a network the operator owns or is authorized to administer:

- Run `scan`.
- Run `wifi` and `net` after connecting.
- Run `dns` against a known hostname.
- Run `probe HOST PORT` against one known service.
- Confirm a failed lookup or closed service returns to the terminal normally.

### Runtime health

Run `health` and `diag` before and after the smoke test.

Check for:

- no unexpected restart,
- stable free heap,
- SD reports `ready` when a card is mounted,
- expected Wi-Fi state,
- responsive keyboard and display.

## Releasing

Production releases are tag-driven.

1. Merge only after CI is green and the physical smoke test passes.
2. Confirm `APP_VERSION` in `src/main.cpp` equals the intended tag version without the leading `v`.
3. Tag the exact tested commit, for example `v0.9`.
4. Push the tag.
5. The Release workflow rebuilds from the tag, reruns native tests, verifies the version, creates SHA-256 checksums, and publishes a GitHub Release with firmware binaries.

The release workflow refuses to publish when the tag and firmware version do not match.

## Rollback

Keep the previous known-good GitHub Release available. If a new firmware build fails physical validation, do not move or recreate its tag. Fix the issue on a new commit and publish a new version.

## Current production posture

Platform Pocket v0.9 is considered a release candidate until:

- CI passes with pinned dependencies,
- the production smoke-test artifact is validated on a real Cardputer ADV,
- storage persistence is verified across a power cycle,
- a tagged release is generated successfully.

After those gates pass, the tested commit can be promoted as the first production-ready v0.9 release.
