//
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/clock/base/ClockServoBase.h"

namespace inet {

void ClockServoBase::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
        clock = getModuleFromPar<SettableClock>(par("clockModule"), this);
}

} // namespace inet

