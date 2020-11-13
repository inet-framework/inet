//
// Copyright (C) 2013 OpenSim Ltd.
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

#include "inet/linklayer/ethernet/common/MacRelayUnit.h"

#include "inet/common/ProtocolTag_m.h"
#include "inet/common/Simsignals.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/linklayer/common/VlanTag_m.h"

namespace inet {

Define_Module(MacRelayUnit);

// FIXME: should handle multicast mac addresses correctly
void MacRelayUnit::handleLowerPacket(Packet *incomingPacket)
{
    auto protocol = incomingPacket->getTag<PacketProtocolTag>()->getProtocol();
    auto macAddressInd = incomingPacket->getTag<MacAddressInd>();
    auto sourceAddress = macAddressInd->getSrcAddress();
    auto destinationAddress = macAddressInd->getDestAddress();
    auto interfaceInd = incomingPacket->getTag<InterfaceInd>();
    int incomingInterfaceId = interfaceInd->getInterfaceId();
    auto incomingInterface = interfaceTable->getInterfaceById(incomingInterfaceId);
    EV_INFO << "Processing packet from network" << EV_FIELD(incomingInterface) << EV_FIELD(incomingPacket) << EV_ENDL;
    updatePeerAddress(incomingInterface, sourceAddress);
    auto outgoingPacket = incomingPacket->dup();
    outgoingPacket->trim();
    outgoingPacket->clearTags();
    outgoingPacket->addTag<PacketProtocolTag>()->setProtocol(protocol);
    if (auto vlanInd = incomingPacket->findTag<VlanInd>())
        outgoingPacket->addTag<VlanReq>()->setVlanId(vlanInd->getVlanId());
    auto& macAddressReq = outgoingPacket->addTag<MacAddressReq>();
    macAddressReq->setSrcAddress(sourceAddress);
    macAddressReq->setDestAddress(destinationAddress);
    if (destinationAddress.isBroadcast())
        broadcastPacket(outgoingPacket, destinationAddress, incomingInterface);
    else {
        // Find output interface of destination address and send packet to output interface
        // if not found then broadcasts to all other interfaces instead
        int outgoingInterfaceId = macAddressTable->getInterfaceIdForAddress(destinationAddress);
        // should not send out the same packet on the same interface
        // (although wireless interfaces are ok to receive the same message)
        if (incomingInterfaceId == outgoingInterfaceId) {
            EV_WARN << "Discarding packet because outgoing interface is the same as incoming interface" << EV_FIELD(destinationAddress) << EV_FIELD(incomingInterface) << EV_FIELD(incomingPacket) << EV_ENDL;
            numDroppedFrames++;
            PacketDropDetails details;
            details.setReason(NO_INTERFACE_FOUND);
            emit(packetDroppedSignal, outgoingPacket, &details);
            delete outgoingPacket;
        }
        else if (outgoingInterfaceId != -1) {
            auto outgoingInterface = interfaceTable->getInterfaceById(outgoingInterfaceId);
            sendPacket(outgoingPacket->dup(), destinationAddress, outgoingInterface);
        }
        else
            broadcastPacket(outgoingPacket, destinationAddress, incomingInterface);
    }
    numProcessedFrames++;
    updateDisplayString();
    delete incomingPacket;
}

} // namespace inet

