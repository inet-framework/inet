//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/queueing/filter/StatisticalRateLimiter.h"

#include "inet/queueing/common/RateTag_m.h"

namespace inet {
namespace queueing {

Define_Module(StatisticalRateLimiter);

void StatisticalRateLimiter::initialize(int stage)
{
    PacketFilterBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        maxDatarate = bps(par("maxDatarate"));
        maxPacketrate = par("maxPacketrate");
    }
}

cGate *StatisticalRateLimiter::getRegistrationForwardingGate(cGate *gate)
{
    if (gate == outputGate)
        return inputGate;
    else if (gate == inputGate)
        return outputGate;
    else
        throw cRuntimeError("Unknown gate");
}

bool StatisticalRateLimiter::canPushPacket(Packet *packet, cGate *gate) const
{
    return consumer == nullptr || consumer->canPushPacket(packet, outputGate->getPathEndGate());
}

bool StatisticalRateLimiter::matchesPacket(const Packet *packet) const
{
    auto rateTag = packet->getTag<RateTag>();
    double p = 0;
    if (!std::isnan(maxDatarate.get()) && rateTag->getDatarate() > maxDatarate)
        p = std::max(p, unit((rateTag->getDatarate() - maxDatarate) / rateTag->getDatarate()).get());
    else if (!std::isnan(maxPacketrate) && rateTag->getPacketrate() > maxPacketrate)
        p = std::max(p, (rateTag->getPacketrate() - maxPacketrate) / rateTag->getPacketrate());
    return p == 0 || dblrand() >= p;
}

} // namespace queueing
} // namespace inet

