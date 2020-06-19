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
#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/ethernet/EtherFrame_m.h"
#include "inet/protocol/ethernet/EthernetCutthroughSource.h"

namespace inet {

Define_Module(EthernetCutthroughSource);

void EthernetCutthroughSource::initialize(int stage)
{
    PacketDestreamer::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        cutthroughOutputGate = gate("cutthroughOut");
        cutthroughConsumer = findConnectedModule<IPassivePacketSink>(cutthroughOutputGate);
        interfaceEntry = getContainingNicModule(this);
        macTable = getModuleFromPar<IMacAddressTable>(par("macTableModule"), this);
        cutthroughTimer = new cMessage("CutthroughTimer");
    }
}

void EthernetCutthroughSource::handleMessage(cMessage *message)
{
    if (message == cutthroughTimer) {
        const auto& header = streamedPacket->peekAtFront<Ieee8023MacAddresses>();
        int interfaceId = macTable->getInterfaceIdForAddress(header->getDest());
        macTable->updateTableWithAddress(interfaceEntry->getInterfaceId(), header->getSrc());
        if (interfaceId != -1 && cutthroughConsumer->canPushPacket(streamedPacket, cutthroughOutputGate)) {
            streamedPacket->trim();
            streamedPacket->addTag<InterfaceReq>()->setInterfaceId(interfaceId);
            streamedPacket->removeTagIfPresent<DispatchProtocolReq>();
            EV_INFO << "Starting streaming packet " << streamedPacket->getName() << "." << std::endl;
            pushOrSendPacketStart(streamedPacket, cutthroughOutputGate, cutthroughConsumer, streamDatarate);
            streamedPacket = nullptr;
            cutthroughInProgress = true;
            updateDisplayString();
        }
        else
            PacketDestreamer::pushPacketStart(streamedPacket, outputGate, streamDatarate);
    }
    else
        PacketDestreamer::handleMessage(message);
}

void EthernetCutthroughSource::pushPacketStart(Packet *packet, cGate *gate, bps datarate)
{
    Enter_Method("pushPacketStart");
    take(packet);
    b cutthroughPosition = PREAMBLE_BYTES + SFD_BYTES + ETHER_MAC_HEADER_BYTES;
    simtime_t delay = s(cutthroughPosition / datarate).get();
    scheduleAt(simTime() + delay, cutthroughTimer);
    delete streamedPacket;
    streamedPacket = packet;
    streamDatarate = datarate;
    updateDisplayString();
}

void EthernetCutthroughSource::pushPacketEnd(Packet *packet, cGate *gate, bps datarate)
{
    if (cutthroughInProgress) {
        Enter_Method("pushPacketEnd");
        take(packet);
        const auto& header = packet->peekAtFront<Ieee8023MacAddresses>();
        macTable->updateTableWithAddress(interfaceEntry->getInterfaceId(), header->getSrc());
        int interfaceId = macTable->getInterfaceIdForAddress(header->getDest());
        packet->trim();
        packet->addTag<InterfaceReq>()->setInterfaceId(interfaceId);
        packet->removeTagIfPresent<DispatchProtocolReq>();
        delete streamedPacket;
        streamedPacket = packet;
        streamDatarate = datarate;
        EV_INFO << "Ending streaming packet " << streamedPacket->getName() << "." << std::endl;
        pushOrSendPacketEnd(streamedPacket, cutthroughOutputGate, cutthroughConsumer, datarate);
        streamedPacket = nullptr;
        cutthroughInProgress = false;
        updateDisplayString();
    }
    else
        PacketDestreamer::pushPacketEnd(packet, gate, datarate);
}

} // namespace inet

