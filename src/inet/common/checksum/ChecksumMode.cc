//
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include <string.h>

#include "inet/common/checksum/ChecksumMode_m.h"

namespace inet {

ChecksumMode parseChecksumMode(const char *checksumModeString, bool allowDisabled)
{
    if (!strcmp(checksumModeString, "disabled")) {
        if (allowDisabled)
            return CHECKSUM_DISABLED;
        else
            throw cRuntimeError("The 'disabled' Checksum mode not allowed");
    }
    else if (!strcmp(checksumModeString, "declared"))
        return CHECKSUM_DECLARED_CORRECT;
    else if (!strcmp(checksumModeString, "declaredCorrect"))
        return CHECKSUM_DECLARED_CORRECT;
    else if (!strcmp(checksumModeString, "declaredIncorrect"))
        return CHECKSUM_DECLARED_INCORRECT;
    else if (!strcmp(checksumModeString, "computed"))
        return CHECKSUM_COMPUTED;
    else
        throw cRuntimeError("Unknown Checksum mode: '%s'", checksumModeString);
}

} // namespace inet
