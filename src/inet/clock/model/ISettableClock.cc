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
    }

} /* namespace inet */
