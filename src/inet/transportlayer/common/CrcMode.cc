//
// Copyright (C) 2017 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include <string.h>

#include "inet/transportlayer/common/CrcMode_m.h"

namespace inet {

CrcMode parseCrcMode(const char *crcModeString, bool allowDisable)
{
    if (!strcmp(crcModeString, "disabled")) {
        if (allowDisable)
            return CRC_DISABLED;
        else
            throw cRuntimeError("The 'disabled' CRC mode not allowed");
    }
    else if (!strcmp(crcModeString, "declared"))
        return CRC_DECLARED_CORRECT;
    else if (!strcmp(crcModeString, "declaredCorrect"))
        return CRC_DECLARED_CORRECT;
    else if (!strcmp(crcModeString, "declaredIncorrect"))
        return CRC_DECLARED_INCORRECT;
    else if (!strcmp(crcModeString, "computed"))
        return CRC_COMPUTED;
    else
        throw cRuntimeError("Unknown CRC mode: '%s'", crcModeString);
}

} // namespace inet

