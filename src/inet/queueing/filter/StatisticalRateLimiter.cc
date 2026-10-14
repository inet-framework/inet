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
        randomValue = dblrand();
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

bool StatisticalRateLimiter::canPushPacket(Packet *packet, const cGate *gate) const
{
    return consumer == nullptr || consumer.canPushPacket(packet);
}

double StatisticalRateLimiter::getDropProbability(const Packet *packet) const
{
    auto rateTag = packet->getTag<RateTag>();
    double p = 0;
    if (!std::isnan(maxDatarate.get()) && rateTag->getDatarate() > maxDatarate)
        p = std::max(p, ((rateTag->getDatarate() - maxDatarate) / rateTag->getDatarate()).get<unit>());
    else if (!std::isnan(maxPacketrate) && rateTag->getPacketrate() > maxPacketrate)
        p = std::max(p, (rateTag->getPacketrate() - maxPacketrate) / rateTag->getPacketrate());
    return p;
}

bool StatisticalRateLimiter::matchesPacket(const Packet *packet) const
{
    double p = getDropProbability(packet);
    return p == 0 || randomValue >= p;
}

void StatisticalRateLimiter::handlePacketFiltered(Packet *packet, bool matches)
{
    // the decision used the random value, so the next decision needs a new one
    if (getDropProbability(packet) != 0)
        randomValue = dblrand();
}

} // namespace queueing
} // namespace inet

