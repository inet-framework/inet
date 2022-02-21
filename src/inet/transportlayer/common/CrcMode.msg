//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;

namespace inet;

enum CrcMode
{
    CRC_MODE_UNDEFINED = -1; // mode is undefined, CRC is not yet set, not serializable
    CRC_DISABLED = 0; // CRC is disabled (not set), serializable
    CRC_DECLARED_CORRECT = 1; // CRC is correctly set without the actual value, not serializable
    CRC_DECLARED_INCORRECT = 2; // the CRC is incorrectly set without the actual value, not serializable
    CRC_COMPUTED = 3; // CRC is set to a value that is potentially incorrect, serializable
}

cplusplus {{
INET_API CrcMode parseCrcMode(const char *crcModeString, bool allowDisable);
}}
