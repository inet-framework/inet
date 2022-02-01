//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/queueing/source/PcapFilePacketProducer.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/Simsignals.h"

namespace inet {
namespace queueing {

Define_Module(PcapFilePacketProducer);

void PcapFilePacketProducer::initialize(int stage)
{
    ActivePacketSourceBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
        pcapReader.openPcap(par("filename"), par("packetNameFormat"));
    else if (stage == INITSTAGE_QUEUEING)
        schedulePacket();
}

void PcapFilePacketProducer::finish()
{
    if (pcapReader.isOpen())
        pcapReader.closePcap();
}

void PcapFilePacketProducer::handleMessage(cMessage *message)
{
    if (message->isPacket()) {
        auto packet = check_and_cast<Packet *>(message);
        if (consumer == nullptr || consumer->canPushPacket(packet, outputGate->getPathEndGate())) {
            emit(packetPushedSignal, packet);
            pushOrSendPacket(packet, outputGate, consumer);
            schedulePacket();
        }
    }
    else
        throw cRuntimeError("Unknown message: %s", message->getFullName());
}

void PcapFilePacketProducer::schedulePacket()
{
    auto pair = pcapReader.readPacket();
    auto packet = pair.second;
    emit(packetCreatedSignal, packet);
    if (packet != nullptr) {
        EV << "Scheduling packet from PCAP file" << EV_FIELD(packet) << EV_ENDL;
        scheduleAt(pair.first, packet);
    }
    else
        EV << "End of PCAP file reached" << EV_ENDL;
}

void PcapFilePacketProducer::handleCanPushPacketChanged(cGate *gate)
{
    Enter_Method("handleCanPushPacketChanged");
    if (gate == outputGate)
        schedulePacket();
}

} // namespace queueing
} // namespace inet

