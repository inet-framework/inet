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

#include "inet/common/INETDefs.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/common/FcsMode_m.h"
#include "inet/linklayer/common/MacAddressTag_m.h"

#ifdef WITH_ETHERNET
#include "inet/linklayer/ethernet/EtherEncap.h"
#include "inet/linklayer/ethernet/EtherPhyFrame_m.h"
#endif // ifdef WITH_ETHERNET

#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/linklayer/ieee80211/portal/Ieee80211Portal.h"
#include "inet/linklayer/ieee8022/Ieee8022LlcHeader_m.h"

namespace inet {

namespace ieee80211 {

Define_Module(Ieee80211Portal);

void Ieee80211Portal::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        upperLayerOutConnected = gate("upperLayerOut")->getPathEndGate()->isConnected();
#ifdef WITH_ETHERNET
        fcsMode = parseFcsMode(par("fcsMode"));
#endif // ifdef WITH_ETHERNET
    }
}

void Ieee80211Portal::handleMessage(cMessage *message)
{
    if (message->arrivedOn("upperLayerIn")) {
        auto packet = check_and_cast<Packet *>(message);
        encapsulate(packet);
        send(packet, "lowerLayerOut");
    }
    else if (message->arrivedOn("lowerLayerIn")) {
        auto packet = check_and_cast<Packet *>(message);
        decapsulate(packet);
        if (upperLayerOutConnected)
            send(packet, "upperLayerOut");
        else
            delete packet;
    }
    else
        throw cRuntimeError("Unknown message");
}

void Ieee80211Portal::encapsulate(Packet *packet)
{
#ifdef WITH_ETHERNET
    auto ethernetHeader = EtherEncap::decapsulateMacHeader(packet);       // do not use const auto& : trimChunks() delete it from packet
    packet->trim();
    packet->addTagIfAbsent<MacAddressReq>()->setDestAddress(ethernetHeader->getDest());
    packet->addTagIfAbsent<MacAddressReq>()->setSrcAddress(ethernetHeader->getSrc());
    if (isIeee8023Header(*ethernetHeader))
        // check that the packet already has an LLC header
        packet->peekAtFront<Ieee8022LlcHeader>();
    else if (isEth2Header(*ethernetHeader)){
        const auto& ieee8022SnapHeader = makeShared<Ieee8022LlcSnapHeader>();
        ieee8022SnapHeader->setOui(0);
        ieee8022SnapHeader->setProtocolId(ethernetHeader->getTypeOrLength());
        packet->insertAtFront(ieee8022SnapHeader);
        packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::ieee8022);
    }
    else
        throw cRuntimeError("Unknown packet: '%s'", packet->getFullName());
#else // ifdef WITH_ETHERNET
    throw cRuntimeError("INET compiled without ETHERNET feature!");
#endif // ifdef WITH_ETHERNET
}

void Ieee80211Portal::decapsulate(Packet *packet)
{
#ifdef WITH_ETHERNET
    packet->trim();
    int typeOrLength = packet->getByteLength();
    const auto& llcHeader = packet->peekAtFront<Ieee8022LlcHeader>();
    if (llcHeader->getSsap() == 0xAA && llcHeader->getDsap() == 0xAA && llcHeader->getControl() == 0x03) {
        const auto& snapHeader = dynamicPtrCast<const Ieee8022LlcSnapHeader>(llcHeader);
        if (snapHeader == nullptr)
            throw cRuntimeError("LLC header indicates SNAP header, but SNAP header is missing");
        if (snapHeader->getOui() == 0) {
            // snap header with ethertype
            typeOrLength = snapHeader->getProtocolId();
            packet->eraseAtFront(snapHeader->getChunkLength());
        }
    }
    const auto& ethernetHeader = makeShared<EthernetMacHeader>();
    ethernetHeader->setSrc(packet->getTag<MacAddressInd>()->getSrcAddress());
    ethernetHeader->setDest(packet->getTag<MacAddressInd>()->getDestAddress());
    ethernetHeader->setTypeOrLength(typeOrLength);
    packet->insertAtFront(ethernetHeader);
    packet->insertAtBack(makeShared<EthernetFcs>(fcsMode));
    packet->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(&Protocol::ethernetMac);
    packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::ethernetMac);
#else // ifdef WITH_ETHERNET
    throw cRuntimeError("INET compiled without ETHERNET feature!");
#endif // ifdef WITH_ETHERNET
}

} // namespace ieee80211

} // namespace inet

