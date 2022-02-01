//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ethernet/common/MacRelayUnit.h"

#include "inet/common/ProtocolTag_m.h"
#include "inet/common/Simsignals.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/linklayer/common/UserPriorityTag_m.h"
#include "inet/linklayer/common/VlanTag_m.h"

namespace inet {

Define_Module(MacRelayUnit);

// FIXME should handle multicast mac addresses correctly
void MacRelayUnit::handleLowerPacket(Packet *incomingPacket)
{
    auto protocol = incomingPacket->getTag<PacketProtocolTag>()->getProtocol();
    auto macAddressInd = incomingPacket->getTag<MacAddressInd>();
    auto sourceAddress = macAddressInd->getSrcAddress();
    auto destinationAddress = macAddressInd->getDestAddress();
    auto interfaceInd = incomingPacket->getTag<InterfaceInd>();
    int incomingInterfaceId = interfaceInd->getInterfaceId();
    auto incomingInterface = interfaceTable->getInterfaceById(incomingInterfaceId);
    unsigned int vlanId = 0;
    if (auto vlanInd = incomingPacket->findTag<VlanInd>())
        vlanId = vlanInd->getVlanId();
    EV_INFO << "Processing packet from network" << EV_FIELD(incomingInterface) << EV_FIELD(incomingPacket) << EV_ENDL;
    updatePeerAddress(incomingInterface, sourceAddress, vlanId);

    auto outgoingPacket = incomingPacket->dup();
    outgoingPacket->trim();
    outgoingPacket->clearTags();
    outgoingPacket->addTag<PacketProtocolTag>()->setProtocol(protocol);
    if (auto vlanInd = incomingPacket->findTag<VlanInd>())
        outgoingPacket->addTag<VlanReq>()->setVlanId(vlanInd->getVlanId());
    if (auto userPriorityInd = incomingPacket->findTag<UserPriorityInd>())
        outgoingPacket->addTag<UserPriorityReq>()->setUserPriority(userPriorityInd->getUserPriority());
    auto& macAddressReq = outgoingPacket->addTag<MacAddressReq>();
    macAddressReq->setSrcAddress(sourceAddress);
    macAddressReq->setDestAddress(destinationAddress);
    if (destinationAddress.isBroadcast())
        broadcastPacket(outgoingPacket, destinationAddress, incomingInterface);
    else if (destinationAddress.isMulticast()) {
        auto outgoingInterfaceIds = macForwardingTable->getMulticastAddressForwardingInterfaces(destinationAddress, vlanId);
        if (outgoingInterfaceIds.size() == 0)
            broadcastPacket(incomingPacket, destinationAddress, incomingInterface);
        else {
            for (auto outgoingInterfaceId : outgoingInterfaceIds) {
                if (interfaceInd != nullptr && outgoingInterfaceId == interfaceInd->getInterfaceId())
                    EV_WARN << "Ignoring outgoing interface because it is the same as incoming interface" << EV_FIELD(destinationAddress) << EV_FIELD(incomingInterface) << EV_FIELD(incomingPacket) << EV_ENDL;
                else {
                    auto outgoingInterface = interfaceTable->getInterfaceById(outgoingInterfaceId);
                    sendPacket(incomingPacket->dup(), destinationAddress, outgoingInterface);
                }
            }
            delete incomingPacket;
        }
    }
    else {
        // Find output interface of destination address and send packet to output interface
        // if not found then broadcasts to all other interfaces instead
        int outgoingInterfaceId = macForwardingTable->getUnicastAddressForwardingInterface(destinationAddress);
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
            sendPacket(outgoingPacket, destinationAddress, outgoingInterface);
        }
        else
            broadcastPacket(outgoingPacket, destinationAddress, incomingInterface);
    }
    numProcessedFrames++;
    updateDisplayString();
    delete incomingPacket;
}

} // namespace inet

