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
        OscillatorBasedClock::initialize(stage);
        if (stage == INITSTAGE_LOCAL) {

        }
    }
    OverdueClockEventHandlingMode getOverdueClockEventHandlingMode(ClockEvent *event){}

    simtime_t handleOverdueClockEvent(ClockEvent *event, simtime_t t){}

    void processCommand(const cXMLElement& node){}

    virtual ppm getOscillatorCompensation() const {}

    void setClockTime(clocktime_t newClockTime){}

    void setClockTime(clocktime_t time, ppm oscillatorCompensation, bool resetOscillator){}
} /* namespace inet */
