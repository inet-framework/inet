//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/queueing/sink/PcapFilePacketConsumer.h"

#include "inet/common/ModuleAccess.h"

namespace inet {
namespace queueing {

Define_Module(PcapFilePacketConsumer);

void PcapFilePacketConsumer::initialize(int stage)
{
    PassivePacketSinkBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        pcapWriter.setFlush(par("alwaysFlush"));
        pcapWriter.open(par("filename"), par("snaplen"));
        networkType = static_cast<PcapLinkType>(par("networkType").intValue());
        const char *dirString = par("direction");
        if (*dirString == 0)
            direction = DIRECTION_UNDEFINED;
        else if (!strcmp(dirString, "outbound"))
            direction = DIRECTION_OUTBOUND;
        else if (!strcmp(dirString, "inbound"))
            direction = DIRECTION_INBOUND;
        else
            throw cRuntimeError("invalid direction parameter value: %s", dirString);
    }
    else if (stage == INITSTAGE_QUEUEING) {
        checkPacketOperationSupport(inputGate);
        if (producer != nullptr)
            producer->handleCanPushPacketChanged(inputGate->getPathStartGate());
    }
}

void PcapFilePacketConsumer::finish()
{
    pcapWriter.close();
}

void PcapFilePacketConsumer::pushPacket(Packet *packet, cGate *gate)
{
    Enter_Method("pushPacket");
    take(packet);
    emit(packetPushedSignal, packet);
    pcapWriter.writePacket(simTime(), packet, direction, getContainingNicModule(this), networkType);
    numProcessedPackets++;
    processedTotalLength += packet->getDataLength();
    delete packet;
}

} // namespace queueing
} // namespace inet

