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
    if (stage == INITSTAGE_LOCAL) {
        std::string filename = getEnvir()->getConfig()->substituteVariables(par("filename"));
        pcapReader.openPcap(filename.c_str(), par("packetNameFormat"));
    }
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
        if (consumer == nullptr || consumer.canPushPacket(packet)) {
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
        // map the pcap record's absolute wall-clock timestamp onto a simulation time;
        // NOTE: this uses the absolute epoch timestamp as-is, so it only works for
        // captures whose timestamps fit into simtime_t -- richer policies (relative to
        // the first record, or to a configured origin) belong here
        auto& time = pair.first;
        scheduleAt(SimTime(time.sec, SIMTIME_S) + SimTime(time.usec, SIMTIME_US), packet);
    }
    else
        EV << "End of PCAP file reached" << EV_ENDL;
}

void PcapFilePacketProducer::handleCanPushPacketChanged(const cGate *gate)
{
    Enter_Method("handleCanPushPacketChanged");
    if (gate == outputGate)
        schedulePacket();
}

} // namespace queueing
} // namespace inet

