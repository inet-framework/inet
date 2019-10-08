//
// Copyright (C) OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see http://www.gnu.org/licenses/.
//

#include "inet/common/ModuleAccess.h"
#include "inet/queueing/PcapPacketProducer.h"

namespace inet {
namespace queueing {

Define_Module(PcapPacketProducer);

void PcapPacketProducer::initialize(int stage)
{
    PacketQueueingElementBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        outputGate = gate("out");
        consumer = dynamic_cast<IPacketConsumer *>(getConnectedModule(outputGate));
        pcapReader.openPcap(par("filename"), par("packetNameFormat"));
    }
    else if (stage == INITSTAGE_QUEUEING) {
        if (consumer != nullptr)
            checkPushPacketSupport(outputGate);
        schedulePacket();
    }
}

void PcapPacketProducer::finish()
{
    if (pcapReader.isOpen())
        pcapReader.closePcap();
}

void PcapPacketProducer::handleMessage(cMessage *message)
{
    if (message->isPacket()) {
        auto packet = check_and_cast<Packet *>(message);
        if (consumer == nullptr || consumer->canPushPacket(packet, outputGate->getPathEndGate())) {
            pushOrSendPacket(packet, outputGate, consumer);
            schedulePacket();
        }
    }
    else
        throw cRuntimeError("Unknown message: %s", message->getFullName());
}

void PcapPacketProducer::schedulePacket()
{
    auto pair = pcapReader.readPacket();
    auto packet = pair.second;
    if (packet != nullptr) {
        EV << "Scheduling packet " << packet->getFullName() << " from PCAP file.\n";
        scheduleAt(pair.first, packet);
    }
    else
        EV << "End of PCAP file reached.\n";
}

void PcapPacketProducer::handleCanPushPacket(cGate *gate)
{
    if (gate->getPathStartGate() == outputGate)
        schedulePacket();
}

} // namespace queueing
} // namespace inet

