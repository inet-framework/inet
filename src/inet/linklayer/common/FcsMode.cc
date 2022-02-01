//
// Copyright (C) 2017 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include <string.h>

#include "inet/linklayer/common/FcsMode_m.h"

namespace inet {

FcsMode parseFcsMode(const char *fcsModeString)
{
    if (!strcmp(fcsModeString, "disabled"))
        return FCS_DISABLED;
    else if (!strcmp(fcsModeString, "declared"))
        return FCS_DECLARED_CORRECT;
    else if (!strcmp(fcsModeString, "declaredCorrect"))
        return FCS_DECLARED_CORRECT;
    else if (!strcmp(fcsModeString, "declaredIncorrect"))
        return FCS_DECLARED_INCORRECT;
    else if (!strcmp(fcsModeString, "computed"))
        return FCS_COMPUTED;
    else
        throw cRuntimeError("Unknown FCS mode: '%s'", fcsModeString);
}

} // namespace inet

