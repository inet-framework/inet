//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee8021d/relay/Ieee8021dRelay.h"

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/common/EtherType_m.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/linklayer/common/VlanTag_m.h"
#include "inet/linklayer/common/UserPriorityTag_m.h"
#include "inet/linklayer/configurator/Ieee8021dInterfaceData.h"

namespace inet {

Define_Module(Ieee8021dRelay);

void Ieee8021dRelay::initialize(int stage)
{
    MacRelayUnitBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        numDispatchedBDPUFrames = numDispatchedNonBPDUFrames = numDeliveredBDPUsToSTP = 0;
        numReceivedBPDUsFromSTP = numReceivedNetworkFrames = 0;

        WATCH(bridgeAddress);
        WATCH(numReceivedNetworkFrames);
        WATCH(numReceivedBPDUsFromSTP);
        WATCH(numDeliveredBDPUsToSTP);
        WATCH(numDispatchedNonBPDUFrames);
    }
    else if (stage == INITSTAGE_LINK_LAYER) {
        registerService(Protocol::ethernetMac, gate("upperLayerIn"), gate("upperLayerOut"));
    }
}

void Ieee8021dRelay::registerAddress(MacAddress mac)
{
    registerAddresses(mac, mac);
}

void Ieee8021dRelay::registerAddresses(MacAddress startMac, MacAddress endMac)
{
    registeredMacAddresses.insert(MacAddressPair(startMac, endMac));
}

void Ieee8021dRelay::handleLowerPacket(Packet *incomingPacket)
{
    numReceivedNetworkFrames++;
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

    const auto& incomingInterfaceData = incomingInterface->findProtocolData<Ieee8021dInterfaceData>();
    // BPDU Handling
    if ((!incomingInterfaceData || incomingInterfaceData->getRole() != Ieee8021dInterfaceData::DISABLED)
            && (destinationAddress == bridgeAddress
                || in_range(registeredMacAddresses, destinationAddress)
                || incomingInterface->matchesMacAddress(destinationAddress))
            && !destinationAddress.isBroadcast())
    {
        EV_DETAIL << "Deliver to upper layer" << endl;
        sendUp(incomingPacket); // deliver to the STP/RSTP module
    }
    else if (incomingInterfaceData && !incomingInterfaceData->isForwarding()) {
        EV_INFO << "Dropping packet because the incoming interface is currently not forwarding" << EV_FIELD(incomingInterface) << EV_FIELD(incomingPacket) << endl;
        numDroppedFrames++;
        PacketDropDetails details;
        details.setReason(NO_INTERFACE_FOUND);
        emit(packetDroppedSignal, incomingPacket, &details);
        delete incomingPacket;
    }
    else {
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

        // TODO revise next "if"s: 2nd drops all packets for me if not forwarding port; 3rd sends up when dest==STP_MULTICAST_ADDRESS; etc.
        // reordering, merge 1st and 3rd, ...

        if (destinationAddress.isBroadcast())
            broadcastPacket(outgoingPacket, destinationAddress, incomingInterface);
        else if (destinationAddress.isMulticast()) {
            auto outgoingInterfaceIds = macForwardingTable->getMulticastAddressForwardingInterfaces(destinationAddress);
            if (outgoingInterfaceIds.size() == 0)
                broadcastPacket(outgoingPacket, destinationAddress, incomingInterface);
            else {
                for (auto outgoingInterfaceId : outgoingInterfaceIds) {
                    auto outgoingInterface = interfaceTable->getInterfaceById(outgoingInterfaceId);
                    if (outgoingInterfaceId != incomingInterfaceId) {
                        if (isForwardingInterface(outgoingInterface))
                            sendPacket(outgoingPacket->dup(), destinationAddress, outgoingInterface);
                        else {
                            EV_WARN << "Discarding packet because output interface is currently not forwarding" << EV_FIELD(outgoingInterface) << EV_FIELD(outgoingPacket) << endl;
                            numDroppedFrames++;
                            PacketDropDetails details;
                            details.setReason(NO_INTERFACE_FOUND);
                            emit(packetDroppedSignal, outgoingPacket, &details);
                        }
                    }
                    else {
                        EV_WARN << "Discarding packet because outgoing interface is the same as incoming interface" << EV_FIELD(destinationAddress) << EV_FIELD(incomingInterface) << EV_FIELD(incomingPacket) << EV_ENDL;
                        numDroppedFrames++;
                        PacketDropDetails details;
                        details.setReason(NO_INTERFACE_FOUND);
                        emit(packetDroppedSignal, outgoingPacket, &details);
                    }
                }
                delete outgoingPacket;
            }
        }
        else {
            auto outgoingInterfaceId = macForwardingTable->getUnicastAddressForwardingInterface(destinationAddress);
            if (outgoingInterfaceId == -1)
                broadcastPacket(outgoingPacket, destinationAddress, incomingInterface);
            else {
                auto outgoingInterface = interfaceTable->getInterfaceById(outgoingInterfaceId);
                if (outgoingInterfaceId != incomingInterfaceId) {
                    if (isForwardingInterface(outgoingInterface))
                        sendPacket(outgoingPacket->dup(), destinationAddress, outgoingInterface);
                    else {
                        EV_WARN << "Discarding packet because output interface is currently not forwarding" << EV_FIELD(outgoingInterface) << EV_FIELD(outgoingPacket) << endl;
                        numDroppedFrames++;
                        PacketDropDetails details;
                        details.setReason(NO_INTERFACE_FOUND);
                        emit(packetDroppedSignal, outgoingPacket, &details);
                    }
                }
                else {
                    EV_WARN << "Discarding packet because outgoing interface is the same as incoming interface" << EV_FIELD(destinationAddress) << EV_FIELD(incomingInterface) << EV_FIELD(incomingPacket) << EV_ENDL;
                    numDroppedFrames++;
                    PacketDropDetails details;
                    details.setReason(NO_INTERFACE_FOUND);
                    emit(packetDroppedSignal, outgoingPacket, &details);
                }
                delete outgoingPacket;
            }
        }
        delete incomingPacket;
    }
    updateDisplayString();
}

void Ieee8021dRelay::handleUpperPacket(Packet *packet)
{
    EV_INFO << "Processing upper packet" << EV_FIELD(packet) << EV_ENDL;
    auto macAddressReq = packet->getTag<MacAddressReq>();
    auto destinationAddress = macAddressReq->getDestAddress();
    auto interfaceReq = packet->findTag<InterfaceReq>();
    if (interfaceReq != nullptr) {
        auto networkInterface = interfaceTable->getInterfaceById(interfaceReq->getInterfaceId());
        sendPacket(packet, destinationAddress, networkInterface);
    }
    else if (destinationAddress.isBroadcast())
        broadcastPacket(packet, destinationAddress, nullptr);
    else if (destinationAddress.isMulticast()) {
        auto outgoingInterfaceIds = macForwardingTable->getMulticastAddressForwardingInterfaces(destinationAddress);
        if (outgoingInterfaceIds.size() == 0)
            broadcastPacket(packet, destinationAddress, nullptr);
        else {
            for (auto outgoingInterfaceId : outgoingInterfaceIds) {
                auto outgoingInterface = interfaceTable->getInterfaceById(outgoingInterfaceId);
                sendPacket(packet->dup(), destinationAddress, outgoingInterface);
            }
            delete packet;
        }
    }
    else {
        int interfaceId = macForwardingTable->getUnicastAddressForwardingInterface(destinationAddress);
        if (interfaceId == -1)
            broadcastPacket(packet, destinationAddress, nullptr);
        else {
            auto networkInterface = interfaceTable->getInterfaceById(interfaceId);
            sendPacket(packet, destinationAddress, networkInterface);
        }
    }
}

bool Ieee8021dRelay::isForwardingInterface(NetworkInterface *networkInterface) const
{
    if (!MacRelayUnitBase::isForwardingInterface(networkInterface))
        return false;
    const auto& interfaceData = networkInterface->findProtocolData<Ieee8021dInterfaceData>();
    return (interfaceData == nullptr || interfaceData->isForwarding());
}

void Ieee8021dRelay::updatePeerAddress(NetworkInterface *incomingInterface, MacAddress sourceAddress, unsigned int vlanId)
{
    const auto& interfaceData = incomingInterface->findProtocolData<Ieee8021dInterfaceData>();
    if (interfaceData == nullptr || interfaceData->isLearning())
        MacRelayUnitBase::updatePeerAddress(incomingInterface, sourceAddress, vlanId);
}

void Ieee8021dRelay::sendUp(Packet *packet)
{
    EV_INFO << "Sending frame to the upper layer" << EV_FIELD(packet) << EV_ENDL;
    send(packet, "upperLayerOut");
}

void Ieee8021dRelay::handleStartOperation(LifecycleOperation *operation)
{
    bridgeNetworkInterface = chooseBridgeInterface();
    if (bridgeNetworkInterface) {
        bridgeAddress = bridgeNetworkInterface->getMacAddress(); // get the bridge's MAC address
        registerAddress(bridgeAddress); // register bridge's MAC address
    }
    else
        throw cRuntimeError("No non-loopback interface found!");
}

void Ieee8021dRelay::handleStopOperation(LifecycleOperation *operation)
{
    bridgeNetworkInterface = nullptr;
}

void Ieee8021dRelay::handleCrashOperation(LifecycleOperation *operation)
{
    bridgeNetworkInterface = nullptr;
}

NetworkInterface *Ieee8021dRelay::chooseBridgeInterface()
{
    for (int i = 0; i < interfaceTable->getNumInterfaces(); i++) {
        NetworkInterface *interface = interfaceTable->getInterface(i);
        if (!interface->isLoopback())
            return interface;
    }
    return nullptr;
}

void Ieee8021dRelay::finish()
{
    MacRelayUnitBase::finish();
    recordScalar("number of received BPDUs from STP module", numReceivedBPDUsFromSTP);
    recordScalar("number of received frames from network (including BPDUs)", numReceivedNetworkFrames);
    recordScalar("number of dropped frames (including BPDUs)", numDroppedFrames);
    recordScalar("number of delivered BPDUs to the STP module", numDeliveredBDPUsToSTP);
    recordScalar("number of dispatched BPDU frames to the network", numDispatchedBDPUFrames);
    recordScalar("number of dispatched non-BDPU frames to the network", numDispatchedNonBPDUFrames);
}

} // namespace inet

