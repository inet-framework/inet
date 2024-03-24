// Copyright (C) 2024 Daniel Zeitler
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "MrpRelay.h"

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/DirectionTag_m.h"
#include "inet/linklayer/common/EtherType_m.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/linklayer/common/VlanTag_m.h"
#include "inet/linklayer/common/UserPriorityTag_m.h"
#include "inet/linklayer/configurator/MrpInterfaceData.h"
#include "inet/linklayer/ethernet/basic/EthernetEncapsulation.h"
#include "inet/linklayer/mrp/common/MrpPdu_m.h"
#include "inet/linklayer/mrp/common/ContinuityCheckMessage_m.h"
#include "inet/linklayer/ieee8021d/common/Ieee8021dInterfaceData.h"
#include <algorithm>
#include <vector>

#include "../mediaredundancy/Mrp.h"

namespace inet {

Define_Module(MrpRelay);

void MrpRelay::initialize(int stage) {
    Ieee8021dRelay::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        numDispatchedMRPFrames = numDispatchedNonMRPFrames = numDeliveredPDUsToMRP = 0;
        numReceivedPDUsFromMRP = numReceivedNetworkFrames = 0;
        mrpMacForwardingTable.reference(this, "macTableModule", true);

        WATCH(numReceivedNetworkFrames);
        WATCH(numReceivedPDUsFromMRP);
        WATCH(numDeliveredPDUsToMRP);
        WATCH(numDispatchedNonMRPFrames);
    } else if (stage == INITSTAGE_LINK_LAYER) {
        registerService(Protocol::ethernetMac, gate("upperLayerIn"), gate("upperLayerOut"));
        bridgeNetworkInterface = chooseBridgeInterface();
        bridgeAddress = bridgeNetworkInterface->getMacAddress();
    }
}

void MrpRelay::handleLowerPacket(Packet *incomingPacket) {
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
    EV_INFO << "Processing packet from network" << EV_FIELD(incomingInterface)
                   << EV_FIELD(incomingPacket) << EV_ENDL;
    updatePeerAddress(incomingInterface, sourceAddress, vlanId);

    const auto &stpData = incomingInterface->findProtocolData<Ieee8021dInterfaceData>();
    const auto &mrpInterfaceData = incomingInterface->findProtocolData<MrpInterfaceData>();

    auto outgoingPacket = incomingPacket->dup();
    outgoingPacket->trim();
    outgoingPacket->clearTags();
    outgoingPacket->addTag<PacketProtocolTag>()->setProtocol(protocol);
    if (auto vlanInd = incomingPacket->findTag<VlanInd>())
        outgoingPacket->addTag<VlanReq>()->setVlanId(vlanInd->getVlanId());
    if (auto userPriorityInd = incomingPacket->findTag<UserPriorityInd>())
        outgoingPacket->addTag<UserPriorityReq>()->setUserPriority(userPriorityInd->getUserPriority());
    auto &macAddressReq = outgoingPacket->addTag<MacAddressReq>();
    macAddressReq->setSrcAddress(sourceAddress);
    macAddressReq->setDestAddress(destinationAddress);

    if (mrpInterfaceData->getRole() != MrpInterfaceData::NOTASSIGNED
            && isMrpMulticast(destinationAddress)) {
        //Mrp-Multicast Handling, forwarding to RingPort according to MrpFilteringDatabase in case incoming interface is not filtering, send up if registered
        auto outgoingInterfaceIds = mrpMacForwardingTable->getMrpForwardingInterfaces(destinationAddress, vlanId);
        if (outgoingInterfaceIds.size() > 0
                && !mrpMacForwardingTable->isMrpIngressFilterInterface(incomingInterfaceId, destinationAddress, vlanId)) {
            EV_DETAIL << "Deliver Mrp-Multicast according to entries in FDB" << EV_ENDL;
            for (auto outgoingInterfaceId : outgoingInterfaceIds) {
                if (interfaceInd != nullptr
                        && outgoingInterfaceId == incomingInterfaceId)
                    EV_DETAIL << "Ignoring outgoing interface because it is the same as incoming interface or currently not forwarding" << EV_FIELD(incomingInterface) << EV_ENDL;
                else {
                    auto outgoingInterface = interfaceTable->getInterfaceById(outgoingInterfaceId);
                    sendPacket(outgoingPacket->dup(), destinationAddress, outgoingInterface);
                }
            }
            numDispatchedMRPFrames++;
        }
        if (in_range(registeredMacAddresses, destinationAddress)) {
            sendUp(incomingPacket);
            numDeliveredPDUsToMRP++;
        } else
            delete incomingPacket;
        delete outgoingPacket;
    } else if ((!stpData
            || stpData->getRole() != Ieee8021dInterfaceData::DISABLED) // STP/RSTP BPDU Handling
            && (destinationAddress == bridgeAddress
                    || in_range(registeredMacAddresses, destinationAddress)
                    || incomingInterface->matchesMacAddress(destinationAddress))
            && !destinationAddress.isBroadcast()) {
        EV_DETAIL << "Deliver to upper layer" << EV_ENDL;
        sendUp(incomingPacket); // deliver to the STP/RSTP module
        delete outgoingPacket;
    } else if (!isForwardingInterface(incomingInterface)) {
        EV_INFO << "Dropping packet because the incoming interface is currently not forwarding"
                << EV_FIELD(incomingInterface)
                << EV_FIELD(incomingPacket)
                << EV_ENDL;
        numDroppedFrames++;
        PacketDropDetails details;
        details.setReason(NO_INTERFACE_FOUND);
        emit(packetDroppedSignal, incomingPacket, &details);
        delete incomingPacket;
        delete outgoingPacket;
    } else {
        // handling Broadcast
        if (destinationAddress.isBroadcast())
            broadcastPacket(outgoingPacket, destinationAddress, incomingInterface);
        //handling Multicast
        else if (destinationAddress.isMulticast()) {
            auto outgoingInterfaceIds = mrpMacForwardingTable->getMulticastAddressForwardingInterfaces(destinationAddress);
            if (outgoingInterfaceIds.size() == 0)
                broadcastPacket(outgoingPacket, destinationAddress, incomingInterface);
            else {
                if (destinationAddress != MacAddress("01:80:C2:00:00:30")
                        || getCcmLevel(incomingPacket) > 0) {
                    for (auto outgoingInterfaceId : outgoingInterfaceIds) {
                        auto outgoingInterface = interfaceTable->getInterfaceById(outgoingInterfaceId);
                        if (interfaceInd != nullptr
                                && outgoingInterfaceId == incomingInterfaceId
                                && !isForwardingInterface(outgoingInterface))
                            EV_DETAIL << "Ignoring outgoing interface because it is the same as incoming interface or currently not forwarding"
                                      << EV_FIELD(destinationAddress)
                                      << EV_FIELD(incomingInterface)
                                      << EV_FIELD(incomingPacket)
                                      << EV_ENDL;
                        else
                            sendPacket(outgoingPacket->dup(), destinationAddress, outgoingInterface);
                    }

                } else {
                    EV_DETAIL << "Discarding packet because ccm-levels <= 0"
                              << EV_FIELD(destinationAddress)
                              << EV_FIELD(incomingInterface)
                              << EV_FIELD(incomingPacket) << EV_ENDL;
                    numDroppedFrames++;
                    PacketDropDetails details;
                    details.setReason(FORWARDING_DISABLED);
                    emit(packetDroppedSignal, outgoingPacket, &details);
                }
                delete outgoingPacket;
            }
        }
        //handling Unicast
        else {
            auto outgoingInterfaceId = mrpMacForwardingTable->getUnicastAddressForwardingInterface(destinationAddress);
            if (outgoingInterfaceId == -1)
                broadcastPacket(outgoingPacket, destinationAddress, incomingInterface);
            else {
                auto outgoingInterface = interfaceTable->getInterfaceById(outgoingInterfaceId);
                if (outgoingInterfaceId != incomingInterfaceId) {
                    if (isForwardingInterface(outgoingInterface))
                        sendPacket(outgoingPacket, destinationAddress, outgoingInterface);
                    else {
                        EV_DETAIL << "Discarding packet because output interface is currently not forwarding"
                                  << EV_FIELD(outgoingInterface)
                                  << EV_FIELD(outgoingPacket)
                                  << EV_ENDL;
                        numDroppedFrames++;
                        PacketDropDetails details;
                        details.setReason(FORWARDING_DISABLED);
                        emit(packetDroppedSignal, outgoingPacket, &details);
                        delete outgoingPacket;
                    }
                } else {
                    EV_DETAIL << "Discarding packet because outgoing interface is the same as incoming interface"
                              << EV_FIELD(destinationAddress)
                              << EV_FIELD(incomingInterface)
                              << EV_FIELD(incomingPacket)
                              << EV_ENDL;
                    numDroppedFrames++;
                    PacketDropDetails details;
                    details.setReason(NO_INTERFACE_FOUND);
                    emit(packetDroppedSignal, outgoingPacket, &details);
                    delete outgoingPacket;
                }
            }
        }
        numDispatchedNonMRPFrames++;
        delete incomingPacket;
    }
    updateDisplayString();
}

bool MrpRelay::isForwardingInterface(NetworkInterface *networkInterface) const {
    const auto &MrpData = networkInterface->findProtocolData<MrpInterfaceData>();
    const auto &Ieee8021dData = networkInterface->findProtocolData<Ieee8021dInterfaceData>();
    if (networkInterface->isLoopback() || !networkInterface->isBroadcast())
        return false;
    else if (!MrpData->isForwarding() || !Ieee8021dData->isForwarding())
        return false;
    else
        return true;
}

int MrpRelay::getCcmLevel(Packet *packet) {
    const auto &ccm = packet->peekAtFront<ContinuityCheckMessage>();
    return ccm->getMdLevel();
}

bool MrpRelay::isMrpMulticast(MacAddress DestinationAddress) {
    if (DestinationAddress.getAddressByte(0) & 0x01
            && DestinationAddress.getAddressByte(1) & 0x15
            && DestinationAddress.getAddressByte(2) & 0x4E)
        return true;
    return false;
}

void MrpRelay::handleUpperPacket(Packet *packet) {
    EV_INFO << "Processing upper packet" << EV_FIELD(packet) << EV_ENDL;
    auto macAddressReq = packet->getTag<MacAddressReq>();
    auto destinationAddress = macAddressReq->getDestAddress();
    auto interfaceReq = packet->findTag<InterfaceReq>();
    auto protocol = packet->getTag<PacketProtocolTag>()->getProtocol();
    if (protocol == &Protocol::ieee8021qCFM || protocol == &Protocol::mrp)
        numReceivedPDUsFromMRP++;
    if (interfaceReq != nullptr) {
        auto networkInterface = interfaceTable->getInterfaceById(interfaceReq->getInterfaceId());
        sendPacket(packet, destinationAddress, networkInterface);
    } else if (destinationAddress.isBroadcast())
        broadcastPacket(packet, destinationAddress, nullptr);
    else if (destinationAddress.isMulticast()) {
        auto outgoingInterfaceIds = mrpMacForwardingTable->getMulticastAddressForwardingInterfaces(destinationAddress);
        if (outgoingInterfaceIds.size() == 0)
            broadcastPacket(packet, destinationAddress, nullptr);
        else {
            for (auto outgoingInterfaceId : outgoingInterfaceIds) {
                auto outgoingInterface = interfaceTable->getInterfaceById(outgoingInterfaceId);
                sendPacket(packet->dup(), destinationAddress, outgoingInterface);
            }
            delete packet;
        }
    } else {
        int interfaceId = mrpMacForwardingTable->getUnicastAddressForwardingInterface(destinationAddress);
        if (interfaceId == -1)
            broadcastPacket(packet, destinationAddress, nullptr);
        else {
            auto networkInterface = interfaceTable->getInterfaceById(interfaceId);
            sendPacket(packet, destinationAddress, networkInterface);
        }
    }
}

void MrpRelay::updatePeerAddress(NetworkInterface *incomingInterface, MacAddress sourceAddress, unsigned int vlanId) {
    EV_INFO << "Learning peer address"
            << EV_FIELD(sourceAddress)
            << EV_FIELD(incomingInterface)
            << EV_ENDL;
    if (!sourceAddress.isMulticast()) {
        if (mrpMacForwardingTable->getUnicastAddressForwardingInterface(sourceAddress, vlanId) == -1) {
            mrpMacForwardingTable->learnUnicastAddressForwardingInterface(incomingInterface->getInterfaceId(), sourceAddress, vlanId);
        }
    } else {
        auto interfaceIds = mrpMacForwardingTable->getMulticastAddressForwardingInterfaces(sourceAddress, vlanId);
        if (std::find(interfaceIds.begin(), interfaceIds.end(), incomingInterface->getInterfaceId()) != interfaceIds.end())
            mrpMacForwardingTable->addMulticastAddressForwardingInterface(incomingInterface->getInterfaceId(), sourceAddress, vlanId);
    }
}

void MrpRelay::sendPacket(Packet *packet, const MacAddress &destinationAddress, NetworkInterface *outgoingInterface) {
    EV_INFO << "Sending packet to peer"
            << EV_FIELD(destinationAddress)
            << EV_FIELD(outgoingInterface)
            << EV_FIELD(packet)
            << EV_ENDL;
    packet->addTagIfAbsent<DirectionTag>()->setDirection(DIRECTION_OUTBOUND);
    packet->addTagIfAbsent<InterfaceReq>()->setInterfaceId(outgoingInterface->getInterfaceId());
    auto protocol = outgoingInterface->getProtocol();
    if (protocol != nullptr)
        packet->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(protocol);
    else
        packet->removeTagIfPresent<DispatchProtocolReq>();
    emit(packetSentToLowerSignal, packet);
    switchingDelay = SimTime(par("switchingDelay").doubleValue(), SIMTIME_US);
    if (packet->findTag<MacAddressReq>() == nullptr) {
        auto &macAddressReq = packet->addTag<MacAddressReq>();
        macAddressReq->setDestAddress(destinationAddress);
        const MacAddress &sourceAddress = outgoingInterface->getMacAddress();
        macAddressReq->setSrcAddress(sourceAddress);
    }
    sendDelayed(packet, switchingDelay, "lowerLayerOut");
}

MacAddress MrpRelay::getBridgeAddress() {
    return this->bridgeAddress;
}

void MrpRelay::handleStartOperation(LifecycleOperation *operation) {

}

void MrpRelay::handleStopOperation(LifecycleOperation *operation) {

}

void MrpRelay::handleCrashOperation(LifecycleOperation *operation) {
    finish();
}

void MrpRelay::finish() {
    Ieee8021dRelay::finish();
    recordScalar("number of received PDUs from MRP module", numReceivedPDUsFromMRP);
    recordScalar("number of received frames from network (including PDUs)", numReceivedNetworkFrames);
    recordScalar("number of dropped frames (including PDUs)", numDroppedFrames);
    recordScalar("number of delivered PDUs to the MRP module", numDeliveredPDUsToMRP);
    recordScalar("number of dispatched MRP frames to the network", numDispatchedMRPFrames);
    recordScalar("number of dispatched non-MRP frames to the network", numDispatchedNonMRPFrames);
}

} // namespace inet

