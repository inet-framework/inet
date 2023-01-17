//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ethernet/common/RelayInterfaceSelector.h"

#include "inet/common/DirectionTag_m.h"
#include "inet/common/ProtocolUtils.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/linklayer/common/VlanTag_m.h"

namespace inet {

Define_Module(RelayInterfaceSelector);

void RelayInterfaceSelector::initialize(int stage)
{
    PacketPusherBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        macForwardingTable.reference(this, "macTableModule", true);
        interfaceTable.reference(this, "interfaceTableModule", true);
        WATCH(numProcessedFrames);
        WATCH(numDroppedFrames);
    }
}

cGate *RelayInterfaceSelector::getRegistrationForwardingGate(cGate *gate)
{
    if (gate == outputGate)
        return inputGate;
    else if (gate == inputGate)
        return outputGate;
    else
        throw cRuntimeError("Unknown gate");
}

void RelayInterfaceSelector::pushPacket(Packet *packet, cGate *gates)
{
    Enter_Method("pushPacket");
    take(packet);
    auto interfaceReq = packet->findTag<InterfaceReq>();
    auto macAddressReq = packet->getTag<MacAddressReq>();
    auto destinationAddress = macAddressReq->getDestAddress();
    if (interfaceReq != nullptr) {
        auto networkInterface = interfaceTable->getInterfaceById(interfaceReq->getInterfaceId());
        sendPacket(packet, destinationAddress, networkInterface);
    }
    else {
        auto interfaceInd = packet->findTag<InterfaceInd>();
        auto incomingInterface = interfaceInd != nullptr ? interfaceTable->getInterfaceById(interfaceInd->getInterfaceId()) : nullptr;
        auto vlanReq = packet->findTag<VlanReq>();
        int vlanId = vlanReq != nullptr ? vlanReq->getVlanId() : 0;
        if (destinationAddress.isBroadcast())
            broadcastPacket(packet, destinationAddress, incomingInterface);
        else if (destinationAddress.isMulticast()) {
            auto outgoingInterfaceIds = macForwardingTable->getMulticastAddressForwardingInterfaces(destinationAddress, vlanId);
            if (outgoingInterfaceIds.size() == 0)
                broadcastPacket(packet, destinationAddress, incomingInterface);
            else {
                for (auto outgoingInterfaceId : outgoingInterfaceIds) {
                    if (interfaceInd != nullptr && outgoingInterfaceId == interfaceInd->getInterfaceId())
                        EV_WARN << "Ignoring outgoing interface because it is the same as incoming interface" << EV_FIELD(destinationAddress) << EV_FIELD(incomingInterface) << EV_FIELD(packet) << EV_ENDL;
                    else {
                        auto outgoingInterface = interfaceTable->getInterfaceById(outgoingInterfaceId);
                        sendPacket(packet->dup(), destinationAddress, outgoingInterface);
                    }
                }
                delete packet;
            }
        }
        else {
            // Find output interface of destination address and send packet to output interface
            // if not found then broadcasts to all other interfaces instead
            int outgoingInterfaceId = macForwardingTable->getUnicastAddressForwardingInterface(destinationAddress, vlanId);
            // should not send out the same packet on the same interface
            // (although wireless interfaces are ok to receive the same message)
            if (interfaceInd != nullptr && outgoingInterfaceId == interfaceInd->getInterfaceId()) {
                EV_WARN << "Discarding packet because outgoing interface is the same as incoming interface" << EV_FIELD(destinationAddress) << EV_FIELD(incomingInterface) << EV_FIELD(packet) << EV_ENDL;
                numDroppedFrames++;
                PacketDropDetails details;
                details.setReason(NO_INTERFACE_FOUND);
                emit(packetDroppedSignal, packet, &details);
                delete packet;
            }
            else if (outgoingInterfaceId != -1) {
                auto outgoingInterface = interfaceTable->getInterfaceById(outgoingInterfaceId);
                sendPacket(packet, destinationAddress, outgoingInterface);
            }
            else
                broadcastPacket(packet, destinationAddress, incomingInterface);
        }
    }
    numProcessedFrames++;
    updateDisplayString();
}

void RelayInterfaceSelector::broadcastPacket(Packet *outgoingPacket, const MacAddress& destinationAddress, NetworkInterface *incomingInterface)
{
    if (incomingInterface == nullptr)
        EV_INFO << "Broadcasting packet to all interfaces" << EV_FIELD(destinationAddress) << EV_FIELD(outgoingPacket) << EV_ENDL;
    else
        EV_INFO << "Broadcasting packet to all interfaces except incoming interface" << EV_FIELD(destinationAddress) << EV_FIELD(incomingInterface) << EV_FIELD(outgoingPacket) << EV_ENDL;
    for (int i = 0; i < interfaceTable->getNumInterfaces(); i++) {
        auto outgoingInterface = interfaceTable->getInterface(i);
        if (incomingInterface != outgoingInterface && isForwardingInterface(outgoingInterface))
            sendPacket(outgoingPacket->dup(), destinationAddress, outgoingInterface);
    }
    delete outgoingPacket;
}

void RelayInterfaceSelector::sendPacket(Packet *packet, const MacAddress& destinationAddress, NetworkInterface *outgoingInterface)
{
    EV_INFO << "Sending packet to peer" << EV_FIELD(destinationAddress) << EV_FIELD(outgoingInterface) << EV_FIELD(packet) << EV_ENDL;
    packet->addTagIfAbsent<DirectionTag>()->setDirection(DIRECTION_OUTBOUND);
    packet->addTagIfAbsent<InterfaceReq>()->setInterfaceId(outgoingInterface->getInterfaceId());
    if (auto outgoingInterfaceProtocol = outgoingInterface->getProtocol())
        ensureEncapsulationProtocolReq(packet, outgoingInterfaceProtocol, true, false);
    setDispatchProtocol(packet);
    pushOrSendPacket(packet, outputGate, consumer);
}

} // namespace inet

