//
// Copyright (C) 2020 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#include "inet/linklayer/ethernet/modular/EthernetCutthroughSource.h"

#include "inet/common/DirectionTag_m.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/ethernet/common/Ethernet.h"
#include "inet/linklayer/ethernet/common/EthernetMacHeader_m.h"

namespace inet {

Define_Module(EthernetCutthroughSource);

void EthernetCutthroughSource::initialize(int stage)
{
    PacketDestreamer::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        cutthroughOutputGate = gate("cutthroughOut");
        cutthroughConsumer = findConnectedModule<IPassivePacketSink>(cutthroughOutputGate);
        networkInterface = getContainingNicModule(this);
        macTable = getModuleFromPar<IMacAddressTable>(par("macTableModule"), this);
        cutthroughTimer = new cMessage("CutthroughTimer");
    }
}

void EthernetCutthroughSource::handleMessage(cMessage *message)
{
    if (message == cutthroughTimer) {
        const auto& header = streamedPacket->peekAtFront<EthernetMacAddressFields>();
        int interfaceId = macTable->getInterfaceIdForAddress(header->getDest());
        macTable->updateTableWithAddress(networkInterface->getInterfaceId(), header->getSrc());
        if (interfaceId != -1 && cutthroughConsumer->canPushPacket(streamedPacket, cutthroughOutputGate)) {
            streamedPacket->trim();
            streamedPacket->addTag<InterfaceReq>()->setInterfaceId(interfaceId);
            streamedPacket->addTagIfAbsent<DirectionTag>()->setDirection(DIRECTION_OUTBOUND);
            streamedPacket->removeTagIfPresent<DispatchProtocolReq>();
            EV_INFO << "Starting streaming packet " << streamedPacket->getName() << "." << std::endl;
            pushOrSendPacketStart(streamedPacket, cutthroughOutputGate, cutthroughConsumer, streamDatarate, streamedPacket->getTransmissionId());
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

void EthernetCutthroughSource::pushPacketEnd(Packet *packet, cGate *gate)
{
    if (cutthroughInProgress) {
        Enter_Method("pushPacketEnd");
        take(packet);
        const auto& header = packet->peekAtFront<EthernetMacAddressFields>();
        macTable->updateTableWithAddress(networkInterface->getInterfaceId(), header->getSrc());
        int interfaceId = macTable->getInterfaceIdForAddress(header->getDest());
        packet->trim();
        packet->addTag<InterfaceReq>()->setInterfaceId(interfaceId);
        packet->addTagIfAbsent<DirectionTag>()->setDirection(DIRECTION_OUTBOUND);
        packet->removeTagIfPresent<DispatchProtocolReq>();
        delete streamedPacket;
        streamedPacket = packet;
        streamDatarate = datarate;
        EV_INFO << "Ending streaming packet " << streamedPacket->getName() << "." << std::endl;
        pushOrSendPacketEnd(streamedPacket, cutthroughOutputGate, cutthroughConsumer, streamedPacket->getTransmissionId());
        streamedPacket = nullptr;
        cutthroughInProgress = false;
        updateDisplayString();
    }
    else
        PacketDestreamer::pushPacketEnd(packet, gate);
}

} // namespace inet

