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

#include "inet/linklayer/ethernet/modular/EthernetMacEncapsulator.h"

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolGroup.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/linklayer/ethernet/common/Ethernet.h"
#include "inet/linklayer/ethernet/common/EthernetMacHeader_m.h"
#include "inet/networklayer/common/NetworkInterface.h"

namespace inet {

Define_Module(EthernetMacEncapsulator);

void EthernetMacEncapsulator::initialize(int stage)
{
    PacketFlowBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
        interfaceTable.reference(this, "interfaceTableModule", true);
    else if (stage == INITSTAGE_LINK_LAYER)
        registerService(Protocol::ethernetMac, inputGate, nullptr);
}

void EthernetMacEncapsulator::processPacket(Packet *packet)
{
    packet->removeTagIfPresent<DispatchProtocolReq>();
    const auto& header = makeShared<EthernetMacHeader>();
    auto macAddressReq = packet->getTag<MacAddressReq>();
    auto srcAddress = macAddressReq->getSrcAddress();
    auto interfaceReq = packet->getTag<InterfaceReq>();
    if (srcAddress.isUnspecified()) {
        if (auto networkInterface = interfaceTable->getInterfaceById(interfaceReq->getInterfaceId()))
            srcAddress = networkInterface->getMacAddress();
    }
    header->setSrc(srcAddress);
    header->setDest(macAddressReq->getDestAddress());
    auto& packetProtocolTag = packet->getTagForUpdate<PacketProtocolTag>();
    auto protocol = packetProtocolTag->getProtocol();
    if (protocol == &Protocol::ieee8022llc)
        header->setTypeOrLength(packet->getByteLength());
    else
        header->setTypeOrLength(ProtocolGroup::ethertype.findProtocolNumber(protocol));
    packet->insertAtFront(header);
    B paddingLength = MIN_ETHERNET_FRAME_BYTES - (packet->getDataLength() + ETHER_FCS_BYTES);
    if (paddingLength > B(0)) {
        const auto& padding = makeShared<ByteCountChunk>(paddingLength, 0);
        packet->insertAtBack(padding);
    }
    const auto& fcs = makeShared<EthernetFcs>();
    fcs->setFcsMode(FCS_COMPUTED);   // TODO FCS_UNFILLED
    fcs->setFcs(0);
    packet->insertAtBack(fcs);
    packetProtocolTag->set(&Protocol::ethernetMac);
}

} // namespace inet

