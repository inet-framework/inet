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

#include "inet/linklayer/ieee8021d/relay/Ieee8021dRelay.h"

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/common/EtherType_m.h"
#include "inet/linklayer/common/Ieee802SapTag_m.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/linklayer/common/VlanTag_m.h"
#include "inet/linklayer/configurator/Ieee8021dInterfaceData.h"
#include "inet/linklayer/ethernet/common/EthernetMacHeader_m.h"
#include "inet/linklayer/ieee8022/Ieee8022LlcHeader_m.h"

namespace inet {

Define_Module(Ieee8021dRelay);

static bool isBpdu(Packet *packet)
{
    auto protocol = packet->getTag<PacketProtocolTag>()->getProtocol();
    if (protocol == &Protocol::ieee8022llc) {
        auto sapInd = packet->findTag<Ieee802SapInd>();
        return sapInd != nullptr && sapInd->getSsap() == 0x42 && sapInd->getDsap() == 0x42; // TODO: && sapInd->getControl() == 3;
    }
    else if (protocol == &Protocol::ethernetMac) {
        const auto& ethernetHeader = packet->peekAtFront<EthernetMacHeader>();
        if (isIeee8023Header(*ethernetHeader)) {
            const auto& llcHeader = packet->peekDataAt<Ieee8022LlcHeader>(ethernetHeader->getChunkLength());
            return (llcHeader->getSsap() == 0x42 && llcHeader->getDsap() == 0x42 && llcHeader->getControl() == 3);
        }
        else
            return false;
    }
    else
        return false;
}

void Ieee8021dRelay::initialize(int stage)
{
    MacRelayUnitBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        isStpAware = par("hasStp");
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
        // TODO FIX Move it at least to STP module (like in ANSA's CDP/LLDP)
        if (isStpAware)
            registerAddress(MacAddress::STP_MULTICAST_ADDRESS);
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
    EV_INFO << "Processing packet from network" << EV_FIELD(incomingInterface) << EV_FIELD(incomingPacket) << EV_ENDL;
    const auto& incomingInterfaceData = incomingInterface->findProtocolData<Ieee8021dInterfaceData>();
    if (isStpAware && incomingInterfaceData == nullptr)
        throw cRuntimeError("Ieee8021dInterfaceData not found for interface %s", incomingInterface->getFullName());
    updatePeerAddress(incomingInterface, sourceAddress);
    // BPDU Handling
    if (isStpAware
            && (destinationAddress == MacAddress::STP_MULTICAST_ADDRESS || destinationAddress == bridgeAddress)
            && incomingInterfaceData->getRole() != Ieee8021dInterfaceData::DISABLED
            && isBpdu(incomingPacket))
    {
        EV_DETAIL << "Deliver BPDU to the STP/RSTP module" << endl;
        sendUp(incomingPacket);    // deliver to the STP/RSTP module
    }
    else if (isStpAware && !incomingInterfaceData->isForwarding()) {
        EV_INFO << "Dropping packet because the incoming interface is currently not forwarding" << EV_FIELD(incomingInterface) << EV_FIELD(incomingPacket) << endl;
        numDroppedFrames++;
        PacketDropDetails details;
        details.setReason(NO_INTERFACE_FOUND);
        emit(packetDroppedSignal, incomingPacket, &details);
        delete incomingPacket;
    }
    else if (in_range(registeredMacAddresses, destinationAddress))
        sendUp(incomingPacket);
    else {
        auto outgoingPacket = incomingPacket->dup();
        outgoingPacket->trim();
        outgoingPacket->clearTags();
        outgoingPacket->addTag<PacketProtocolTag>()->setProtocol(protocol);
        if (auto vlanInd = incomingPacket->findTag<VlanInd>())
            outgoingPacket->addTag<VlanReq>()->setVlanId(vlanInd->getVlanId());
        auto& macAddressReq = outgoingPacket->addTag<MacAddressReq>();
        macAddressReq->setSrcAddress(sourceAddress);
        macAddressReq->setDestAddress(destinationAddress);

        //TODO revise next "if"s: 2nd drops all packets for me if not forwarding port; 3rd sends up when dest==STP_MULTICAST_ADDRESS; etc.
        // reordering, merge 1st and 3rd, ...

        if (destinationAddress.isBroadcast())
            broadcastPacket(outgoingPacket, destinationAddress, incomingInterface);
        else {
            int outgoingInterfaceId = macAddressTable->getInterfaceIdForAddress(destinationAddress);
            if (outgoingInterfaceId == -1)
                broadcastPacket(outgoingPacket, destinationAddress, incomingInterface);
            else {
                auto outgoingInterface = interfaceTable->getInterfaceById(outgoingInterfaceId);
                if (outgoingInterfaceId != incomingInterfaceId) {
                    if (isForwardingInterface(outgoingInterface))
                        sendPacket(outgoingPacket, destinationAddress, outgoingInterface);
                    else {
                        EV_WARN << "Discarding packet because output interface is currently not forwarding" << EV_FIELD(outgoingInterface) << EV_FIELD(outgoingPacket) << endl;
                        numDroppedFrames++;
                        PacketDropDetails details;
                        details.setReason(NO_INTERFACE_FOUND);
                        emit(packetDroppedSignal, outgoingPacket, &details);
                        delete outgoingPacket;
                    }
                }
                else {
                    EV_WARN << "Discarding packet because outgoing interface is the same as incoming interface" << EV_FIELD(destinationAddress) << EV_FIELD(incomingInterface) << EV_FIELD(incomingPacket) << EV_ENDL;
                    numDroppedFrames++;
                    PacketDropDetails details;
                    details.setReason(NO_INTERFACE_FOUND);
                    emit(packetDroppedSignal, outgoingPacket, &details);
                    delete outgoingPacket;
                }
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
    else {
        int interfaceId = macAddressTable->getInterfaceIdForAddress(destinationAddress);
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
    return MacRelayUnitBase::isForwardingInterface(networkInterface) &&
           (!isStpAware || networkInterface->getProtocolData<Ieee8021dInterfaceData>()->isForwarding());
}

void Ieee8021dRelay::updatePeerAddress(NetworkInterface *incomingInterface, MacAddress sourceAddress)
{
    if (!isStpAware || incomingInterface->getProtocolData<Ieee8021dInterfaceData>()->isLearning())
        MacRelayUnitBase::updatePeerAddress(incomingInterface, sourceAddress);
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
    // TODO: Currently, we assume that the first non-loopback interface is an Ethernet interface
    //       since relays work on EtherSwitches.
    //       NOTE that, we don't check if the returning interface is an Ethernet interface!
    for (int i = 0; i < interfaceTable->getNumInterfaces(); i++) {
        NetworkInterface *current = interfaceTable->getInterface(i);
        if (!current->isLoopback())
            return current;
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

