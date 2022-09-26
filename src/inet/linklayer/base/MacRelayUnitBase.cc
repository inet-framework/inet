//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/base/MacRelayUnitBase.h"

#include "inet/common/DirectionTag_m.h"
#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ProtocolGroup.h"
#include "inet/common/StringFormat.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/networklayer/common/NetworkInterface.h"

namespace inet {

Define_Module(MacRelayUnitBase);

void MacRelayUnitBase::initialize(int stage)
{
    LayeredProtocolBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        macForwardingTable.reference(this, "macTableModule", true);
        interfaceTable.reference(this, "interfaceTableModule", true);
        numProcessedFrames = numDroppedFrames = 0;
        WATCH(numProcessedFrames);
        WATCH(numDroppedFrames);
    }
    else if (stage == INITSTAGE_NETWORK_LAYER) {
        registerAnyService(gate("upperLayerIn"), gate("upperLayerOut"));
        registerAnyProtocol(gate("lowerLayerOut"), gate("lowerLayerIn"));
    }
}

std::string MacRelayUnitBase::resolveDirective(char directive) const
{
    switch (directive) {
        case 'p':
            return std::to_string(numProcessedFrames);
        case 'd':
            return std::to_string(numDroppedFrames);
        default:
            throw cRuntimeError("Unknown directive: %c", directive);
    }
}

void MacRelayUnitBase::updateDisplayString() const
{
    if (getEnvir()->isGUI()) {
        auto text = StringFormat::formatString(par("displayStringTextFormat"), this);
        getDisplayString().setTagArg("t", 0, text.c_str());
    }
}

void MacRelayUnitBase::broadcastPacket(Packet *outgoingPacket, const MacAddress& destinationAddress, NetworkInterface *incomingInterface)
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

void MacRelayUnitBase::sendPacket(Packet *packet, const MacAddress& destinationAddress, NetworkInterface *outgoingInterface)
{
    EV_INFO << "Sending packet to peer" << EV_FIELD(destinationAddress) << EV_FIELD(outgoingInterface) << EV_FIELD(packet) << EV_ENDL;
    packet->addTagIfAbsent<DirectionTag>()->setDirection(DIRECTION_OUTBOUND);
    packet->addTagIfAbsent<InterfaceReq>()->setInterfaceId(outgoingInterface->getInterfaceId());
    auto protocol = outgoingInterface->getProtocol();
    if (protocol != nullptr)
        packet->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(protocol);
    else
        packet->removeTagIfPresent<DispatchProtocolReq>();
    emit(packetSentToLowerSignal, packet);
    send(packet, "lowerLayerOut");
}

void MacRelayUnitBase::updatePeerAddress(NetworkInterface *incomingInterface, MacAddress sourceAddress, unsigned int vlanId)
{
    if (!sourceAddress.isMulticast()) {
        EV_INFO << "Learning peer address" << EV_FIELD(sourceAddress) << EV_FIELD(incomingInterface) << EV_ENDL;
        macForwardingTable->learnUnicastAddressForwardingInterface(incomingInterface->getInterfaceId(), sourceAddress, vlanId);
    }
}

void MacRelayUnitBase::finish()
{
    recordScalar("processed frames", numProcessedFrames);
    recordScalar("discarded frames", numDroppedFrames);
}

} // namespace inet

