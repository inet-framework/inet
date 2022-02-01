//
// Copyright (C) 2021 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/clock/common/ClockEvent.h"

#include "inet/clock/contract/IClock.h"

namespace inet {

Register_Class(ClockEvent)

void ClockEvent::execute()
{
    if (clock != nullptr)
        clock->handleClockEvent(this);
    else {
        // TODO: this should be part of setArrival if clock is nullptr
        arrivalClockTime = SIMTIME_AS_CLOCKTIME(getArrivalTime());
        cMessage::execute();
    }
}

} // namespace inet

