//
// Copyright (C) 2020 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#include "inet/protocolelement/shaper/EligibilityTimeFilter.h"

#include "inet/protocolelement/shaper/EligibilityTimeTag_m.h"

namespace inet {

Define_Module(EligibilityTimeFilter);

void EligibilityTimeFilter::initialize(int stage)
{
    PacketFilterBase::initialize(stage);
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
    simtime_t arrivalTime = simTime();
    const auto& eligibilityTimeTag = packet->findTag<EligibilityTimeTag>();
    return eligibilityTimeTag != nullptr && (maxResidenceTime == -1 || eligibilityTimeTag->getEligibilityTime() <= arrivalTime + maxResidenceTime);
}

} // namespace inet

