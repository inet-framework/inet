//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/protocolelement/shaper/EligibilityTimeFilter.h"

#include "inet/protocolelement/shaper/EligibilityTimeTag_m.h"

namespace inet {

Define_Module(EligibilityTimeFilter);

void EligibilityTimeFilter::initialize(int stage)
{
    ClockUserModuleMixin::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
        maxResidenceTime = par("maxResidenceTime");
}

cGate *EligibilityTimeFilter::getRegistrationForwardingGate(cGate *gate)
{
    if (gate == outputGate)
        return inputGate;
    else if (gate == inputGate)
        return outputGate;
    else
        throw cRuntimeError("Unknown gate");
}

bool EligibilityTimeFilter::matchesPacket(const Packet *packet) const
{
    clocktime_t arrivalTime = getClockTime();
    const auto& eligibilityTimeTag = packet->findTag<EligibilityTimeTag>();
    return eligibilityTimeTag != nullptr && (maxResidenceTime == -1 || eligibilityTimeTag->getEligibilityTime() <= arrivalTime + maxResidenceTime);
}

} // namespace inet

