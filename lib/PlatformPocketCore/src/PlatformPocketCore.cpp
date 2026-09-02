// PlatformPocketCore.cpp

#include "PlatformPocketCore.h"

/**
 * @file PlatformPocketCore.cpp
 * @brief Implementations of hardware-independent Platform Pocket helpers.
 */

const char *getSignalLabel(int rssi)
{
    if (rssi >= -50) return "STRONG";
    if (rssi >= -65) return "GOOD";
    if (rssi >= -75) return "FAIR";
    return "WEAK";
}
