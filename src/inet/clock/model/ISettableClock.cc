//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/clock/model/ISettableClock.h"

Define_Module(ISettableClock);

namespace inet {

    void ISettableClock::initialize(int stage)
    {
        ISettableClock::initialize(stage);
        if (stage == INITSTAGE_LOCAL) {

        }
    }

    void setClockTime(clocktime_t newClockTime){}

} /* namespace inet */
