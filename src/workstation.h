#pragma once

#include <Arduino.h>

namespace PocketWorkstation
{
/** @brief Initialize offline workstation content on the SD workspace. */
void begin();

/** @brief List useful files from the Platform Pocket workspace. */
String fileManagerSummary();

/** @brief Normalize a user-supplied editor filename to a safe .md/.txt basename. */
String normalizeEditorFileName(const String &name);

/** @brief List editable .md/.txt files in the notes workspace. */
String editorFileSummary();

/** @brief Save text to a named notes workspace file. */
bool saveEditorFile(const String &name, const String &text);

/** @brief Load a bounded named notes workspace file. */
String loadEditorFile(const String &name, size_t limit = 8192);

/** @brief Save Markdown text to the legacy default editor file. */
bool saveEditorNote(const String &text);

/** @brief Load the legacy default editor file. */
String loadEditorNote();

/** @brief Create a new incident notebook entry. */
String createIncident(const String &title);

/** @brief Append a line to the latest incident notebook. */
bool appendIncident(const String &text);

/** @brief Summarize the incident workspace. */
String incidentSummary();

/** @brief Compare the two newest diagnostic snapshots. */
String compareSnapshots();

/** @brief List bundled offline runbooks. */
String runbookSummary();

/** @brief Read a bundled runbook by short name. */
String readRunbook(const String &name);

/** @brief Return offline troubleshooting guidance for a symptom. */
String troubleshoot(const String &symptom);
} // namespace PocketWorkstation
