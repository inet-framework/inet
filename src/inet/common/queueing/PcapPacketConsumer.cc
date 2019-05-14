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
#include "inet/common/queueing/PcapPacketConsumer.h"

namespace inet {
namespace queueing {

Define_Module(PcapPacketConsumer);

void PcapPacketConsumer::initialize(int stage)
{
    PacketSinkBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        inputGate = gate("in");
        producer = dynamic_cast<IPacketProducer *>(getConnectedModule(inputGate));
        pcapWriter.setFlushParameter(par("alwaysFlush").boolValue());
        pcapWriter.openPcap(par("filename"), par("snaplen"), par("networkType"));
    }
    else if (stage == INITSTAGE_QUEUEING) {
        if (producer != nullptr) {
            checkPushPacketSupport(inputGate);
            producer->handleCanPushPacket(inputGate);
        }
    }
}

void PcapPacketConsumer::finish()
{
    pcapWriter.closePcap();
}

void PcapPacketConsumer::pushPacket(Packet *packet, cGate *gate)
{
    EV_INFO << "Writing packet " << packet->getName() << " to PCAP file." << endl;
    pcapWriter.writePacket(simTime(), packet);
    numProcessedPackets++;
    processedTotalLength += packet->getDataLength();
    delete packet;
}

} // namespace queueing
} // namespace inet

