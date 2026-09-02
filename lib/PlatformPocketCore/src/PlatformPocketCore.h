// PlatformPocketCore.h

#pragma once

/**
 * @file PlatformPocketCore.h
 * @brief Shared, hardware-independent helpers for Platform Pocket.
 *
 * Keep functions in this core library deterministic where practical so they
 * can be exercised by the native PlatformIO test environment without Cardputer
 * hardware attached.
 */

/**
 * @brief Convert Wi-Fi RSSI into a compact signal-quality label.
 *
 * The thresholds are intended for operator-facing UI feedback rather than
 * radio calibration or RF engineering measurements.
 *
 * @param rssi Received signal strength in dBm, typically a negative value.
 * @return A static label: "STRONG", "GOOD", "FAIR", or "WEAK".
 */
const char *getSignalLabel(int rssi);
