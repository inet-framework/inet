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
#include "inet/queueing/sink/PcapFilePacketConsumer.h"

namespace inet {
namespace queueing {

Define_Module(PcapFilePacketConsumer);

void PcapFilePacketConsumer::initialize(int stage)
{
    PacketSinkBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        inputGate = gate("in");
        producer = findConnectedModule<IActivePacketSource>(inputGate);
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
            producer->handleCanPushPacket(inputGate->getPathStartGate());
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
    EV_INFO << "Writing packet " << packet->getName() << " to PCAP file." << endl;
    emit(packetPushedSignal, packet);
    pcapWriter.writePacket(simTime(), packet, direction, getContainingNicModule(this), networkType);
    numProcessedPackets++;
    processedTotalLength += packet->getDataLength();
    delete packet;
}

} // namespace queueing
} // namespace inet

