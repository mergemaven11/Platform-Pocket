#pragma once

#include <Arduino.h>

namespace PocketStorage
{
/**
 * @brief Initialize the Cardputer ADV microSD card and Platform Pocket workspace.
 *
 * @return true when the card is mounted and the workspace is available.
 */
bool begin();

/**
 * @brief Report whether persistent SD storage is ready for use.
 *
 * @return true when the SD card is mounted and initialized.
 */
bool ready();

/**
 * @brief Build a concise human-readable SD card health summary.
 *
 * @return Storage type, capacity, usage, and workspace status.
 */
String statusText();

/**
 * @brief Append a quick Markdown note to the persistent notes inbox.
 *
 * @param text Note text entered by the operator.
 * @return true when the note was written successfully.
 */
bool appendQuickNote(const String &text);

/**
 * @brief Summarize the persistent quick-note inbox.
 *
 * @return Note file location and current byte size, or an offline message.
 */
String noteSummary();

/**
 * @brief Save a diagnostic snapshot to the SD workspace.
 *
 * @return Snapshot path on success or an error message prefixed with an exclamation mark.
 */
String saveDiagnosticSnapshot();

/**
 * @brief Describe the standard Platform Pocket workspace directories.
 *
 * @return Compact directory map suitable for the handheld UI or terminal.
 */
String workspaceSummary();
} // namespace PocketStorage
