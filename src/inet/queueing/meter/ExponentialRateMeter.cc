//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/queueing/meter/ExponentialRateMeter.h"

#include "inet/common/ModuleAccess.h"
#include "inet/queueing/common/RateTag_m.h"

namespace inet {
namespace queueing {

Define_Module(ExponentialRateMeter);

void ExponentialRateMeter::initialize(int stage)
{
    PacketMeterBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        alpha = par("alpha");
        WATCH(packetrate);
        WATCH(datarate);
    }
}

void ExponentialRateMeter::meterPacket(Packet *packet)
{
    auto now = simTime();
    if (now != lastUpdate) {
        auto elapsedTime = (now - lastUpdate).dbl();
        auto packetrateChange = (currentNumPackets / elapsedTime) - packetrate;
        packetrate = packetrate + packetrateChange * alpha;
        auto datarateChange = (currentTotalPacketLength / s(elapsedTime)) - datarate;
        datarate = datarate + datarateChange * alpha;
        currentNumPackets = 0;
        currentTotalPacketLength = b(0);
        lastUpdate = now;
    }
    currentNumPackets++;
    currentTotalPacketLength += packet->getTotalLength();
    auto rateTag = packet->addTagIfAbsent<RateTag>();
    rateTag->setDatarate(datarate);
    rateTag->setPacketrate(packetrate);
}

} // namespace queueing
} // namespace inet

