//
// Copyright (C) 2012 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/networklayer/common/ModuleIdAddress.h"

namespace inet {

bool ModuleIdAddress::tryParse(const char *addr)
{
    char *endp;
    id = strtol(addr, &endp, 10);
    return *endp == 0;
}

} // namespace inet

