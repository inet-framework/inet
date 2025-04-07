//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;

namespace inet;

enum ChecksumMode
{
    CHECKSUM_MODE_UNDEFINED = -1; // mode is undefined, checksum is not yet set, not serializable
    CHECKSUM_DISABLED = 0; // checksum is disabled (not set), serializable
    CHECKSUM_DECLARED_CORRECT = 1; // checksum is correctly set without the actual value, not serializable
    CHECKSUM_DECLARED_INCORRECT = 2; // the checksum is incorrectly set without the actual value, not serializable
    CHECKSUM_COMPUTED = 3; // checksum is set to a value that is potentially incorrect, serializable
}

cplusplus {{
INET_API ChecksumMode parseChecksumMode(const char *checksumModeString, bool allowDisabled=true);
}}
