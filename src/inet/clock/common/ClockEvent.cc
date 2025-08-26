//
// Copyright (C) 2021 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/clock/common/ClockEvent.h"

#include "inet/clock/contract/IClock.h"
#include "inet/common/IPrintableObject.h"

namespace inet {

Register_Class(ClockEvent)

void ClockEvent::execute()
{
    cSimpleModule *targetModule = check_and_cast<cSimpleModule *>(getTargetObject());
    cContextSwitcher contextSwitcher(targetModule);
    if (clock != nullptr) {
        clocktime_t clockTime = clock->getClockTime();
        EV_DEBUG << "Executing clock event" << EV_FIELD(clockTime) << EV_FIELD(event, this) << EV_ENDL;
        // NOTE: IClock interface 2. invariant
        ASSERT(getArrivalClockTime() == clockTime);
        clock->handleClockEvent(this);
    }
    else {
        // TODO: this should be part of setArrival if clock is nullptr
        arrivalClockTime = SIMTIME_AS_CLOCKTIME(getArrivalTime());
        cMessage::execute();
    }
}

} // namespace inet

