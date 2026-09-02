#pragma once

#include <Arduino.h>

namespace PocketWorkstation
{
/** @brief Initialize offline workstation content on the SD workspace. */
void begin();

/** @brief List useful files from the Platform Pocket workspace. */
String fileManagerSummary();

/** @brief Save Markdown text from the full-screen editor. */
bool saveEditorNote(const String &text);

/** @brief Load the current editor note. */
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
