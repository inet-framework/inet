//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/protocolelement/common/HeaderPosition.h"

namespace inet {

HeaderPosition parseHeaderPosition(const char *string)
{
    if (!strcmp(string, "") || !strcmp(string, "none"))
        return HP_NONE;
    else if (!strcmp(string, "front"))
        return HP_FRONT;
    else if (!strcmp(string, "back"))
        return HP_BACK;
    else
        throw cRuntimeError("Unknown header position value");
}

} // namespace inet

