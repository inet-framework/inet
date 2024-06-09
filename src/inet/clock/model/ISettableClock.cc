//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/clock/model/ISettableClock.h"

namespace inet {


    Define_Module(ISettableClock);

    void ISettableClock::initialize(int stage)
    {
        OscillatorBasedClock::initialize(stage);
        if (stage == INITSTAGE_LOCAL) {
            const char *text = par("defaultOverdueClockEventHandlingMode");
            if (!strcmp(text, "execute"))
                defaultOverdueClockEventHandlingMode = EXECUTE;
            else if (!strcmp(text, "skip"))
                defaultOverdueClockEventHandlingMode = SKIP;
            else if (!strcmp(text, "error"))
                defaultOverdueClockEventHandlingMode = ERROR;
            else
                throw cRuntimeError("Unknown defaultOverdueClockEventHandlingMode parameter value");
        }
    }

} /* namespace inet */
