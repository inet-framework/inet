//
// Copyright (C) 2013 OpenSim Ltd
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
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//
// author: Zoltan Bojthe
//

#include <stdio.h>
#include <string.h>
#include "inet/common/INETUtils.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/ProtocolGroup.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/queue/IPassiveQueue.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/linklayer/ideal/MultipleAccessMac.h"
#include "inet/linklayer/ideal/IdealMacHeader_m.h"
#include "inet/networklayer/contract/IInterfaceTable.h"

namespace inet {

using namespace inet::physicallayer;

Define_Module(MultipleAccessMac);

MultipleAccessMac::MultipleAccessMac()
{
}

MultipleAccessMac::~MultipleAccessMac()
{
    delete lastSentPk;
    cancelAndDelete(ackTimeoutMsg);
}

void MultipleAccessMac::flushQueue()
{
    ASSERT(queueModule);
    while (!queueModule->isEmpty()) {
        cMessage *msg = queueModule->pop();
        PacketDropDetails details;
        details.setReason(INTERFACE_DOWN);
        emit(packetDropSignal, msg, &details);
        delete msg;
    }
    queueModule->clear();    // clear request count
}

void MultipleAccessMac::clearQueue()
{
    ASSERT(queueModule);
    queueModule->clear();
}

void MultipleAccessMac::initialize(int stage)
{
    MacProtocolBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        outStandingRequests = 0;

        bitrate = par("bitrate").doubleValue();
        headerLength = par("headerLength");
        promiscuous = par("promiscuous");
        fullDuplex = par("fullDuplex");
        useAck = par("useAck");
        ackTimeout = par("ackTimeout");

        cModule *radioModule = gate("lowerLayerOut")->getPathEndGate()->getOwnerModule();
        radioModule->subscribe(IRadio::transmissionStateChangedSignal, this);
        radio = check_and_cast<IRadio *>(radioModule);
        transmissionState = IRadio::TRANSMISSION_STATE_UNDEFINED;

        // find queueModule
        cGate *queueOut = gate("upperLayerIn")->getPathStartGate();
        queueModule = dynamic_cast<IPassiveQueue *>(queueOut->getOwnerModule());
        if (!queueModule)
            throw cRuntimeError("Missing queueModule");

        initializeMACAddress();
    }
    else if (stage == INITSTAGE_LINK_LAYER) {
        radio->setRadioMode(fullDuplex ? IRadio::RADIO_MODE_TRANSCEIVER : IRadio::RADIO_MODE_RECEIVER);
        if (useAck)
            ackTimeoutMsg = new cMessage("link-break");
        getNextMsgFromHL();
        registerInterface();
    }
}

void MultipleAccessMac::initializeMACAddress()
{
    const char *addrstr = par("address");

    if (!strcmp(addrstr, "auto")) {
        // assign automatic address
        address = MacAddress::generateAutoAddress();

        // change module parameter from "auto" to concrete address
        par("address").setStringValue(address.str().c_str());
    }
    else {
        address.setAddress(addrstr);
    }
}

InterfaceEntry *MultipleAccessMac::createInterfaceEntry()
{
    InterfaceEntry *e = getContainingNicModule(this);

    // data rate
    e->setDatarate(bitrate);

    // generate a link-layer address to be used as interface token for IPv6
    e->setMACAddress(address);
    e->setInterfaceToken(address.formInterfaceIdentifier());

    // MTU: typical values are 576 (Internet de facto), 1500 (Ethernet-friendly),
    // 4000 (on some point-to-point links), 4470 (Cisco routers default, FDDI compatible)
    e->setMtu(par("mtu"));

    // capabilities
    e->setMulticast(true);
    e->setBroadcast(true);

    return e;
}

void MultipleAccessMac::receiveSignal(cComponent *source, simsignal_t signalID, long value, cObject *details)
{
    Enter_Method_Silent();
    if (signalID == IRadio::transmissionStateChangedSignal) {
        IRadio::TransmissionState newRadioTransmissionState = (IRadio::TransmissionState)value;
        if (transmissionState == IRadio::TRANSMISSION_STATE_TRANSMITTING && newRadioTransmissionState == IRadio::TRANSMISSION_STATE_IDLE) {
            radio->setRadioMode(fullDuplex ? IRadio::RADIO_MODE_TRANSCEIVER : IRadio::RADIO_MODE_RECEIVER);
            if (!lastSentPk)
                getNextMsgFromHL();
        }
        transmissionState = newRadioTransmissionState;
    }
}

void MultipleAccessMac::startTransmitting(Packet *msg)
{
    // if there's any control info, remove it; then encapsulate the packet
    if (lastSentPk)
        throw cRuntimeError("Model error: unacked send");
    MacAddress dest = msg->getMandatoryTag<MacAddressReq>()->getDestAddress();
    encapsulate(check_and_cast<Packet *>(msg));

    if (!dest.isBroadcast() && !dest.isMulticast() && !dest.isUnspecified()) {    // unicast
        if (useAck) {
            lastSentPk = msg->dup();
            scheduleAt(simTime() + ackTimeout, ackTimeoutMsg);
        }
    }

    // send
    EV << "Starting transmission of " << msg << endl;
    radio->setRadioMode(fullDuplex ? IRadio::RADIO_MODE_TRANSCEIVER : IRadio::RADIO_MODE_TRANSMITTER);
    sendDown(msg);
}

void MultipleAccessMac::getNextMsgFromHL()
{
    ASSERT(outStandingRequests >= queueModule->getNumPendingRequests());
    if (outStandingRequests == 0) {
        queueModule->requestPacket();
        outStandingRequests++;
    }
    ASSERT(outStandingRequests <= 1);
}

void MultipleAccessMac::handleUpperPacket(Packet *packet)
{
    outStandingRequests--;
    if (radio->getTransmissionState() == IRadio::TRANSMISSION_STATE_TRANSMITTING) {
        // Logic error: we do not request packet from the external queue when radio is transmitting
        throw cRuntimeError("Received msg for transmission but transmitter is busy");
    }
    else {
        // We are idle, so we can start transmitting right away.
        EV << "Received " << packet << " for transmission\n";
        startTransmitting(packet);
    }
}

void MultipleAccessMac::handleLowerPacket(Packet *packet)
{
    auto idealMacHeader = packet->peekHeader<IdealMacHeader>();
    if (packet->hasBitError()) {
        EV << "Received frame '" << packet->getName() << "' contains bit errors or collision, dropping it\n";
        PacketDropDetails details;
        details.setReason(INCORRECTLY_RECEIVED);
        emit(packetDropSignal, packet, &details);
        delete packet;
        return;
    }

    if (!dropFrameNotForUs(packet)) {
        int senderModuleId = idealMacHeader->getSrcModuleId();
        MultipleAccessMac *senderMac = dynamic_cast<MultipleAccessMac *>(getSimulation()->getModule(senderModuleId));
        // TODO: this whole out of bounds ack mechanism is fishy
        if (senderMac && senderMac->useAck)
            senderMac->acked(packet);
        // decapsulate and attach control info
        decapsulate(packet);
        EV << "Passing up contained packet '" << packet->getName() << "' to higher layer\n";
        sendUp(packet);
    }
}

void MultipleAccessMac::handleSelfMessage(cMessage *message)
{
    if (message == ackTimeoutMsg) {
        EV_DETAIL << "MultipleAccessMac: timeout: " << lastSentPk->getFullName() << " is lost\n";
        auto idealMacHeader = lastSentPk->popHeader<IdealMacHeader>();
        lastSentPk->ensureTag<PacketProtocolTag>()->setProtocol(ProtocolGroup::ethertype.getProtocol(idealMacHeader->getNetworkProtocol()));
        // packet lost
        emit(linkBreakSignal, lastSentPk);
        delete lastSentPk;
        lastSentPk = nullptr;
        getNextMsgFromHL();
    }
    else {
        MacProtocolBase::handleSelfMessage(message);
    }
}

void MultipleAccessMac::acked(Packet *frame)
{
    Enter_Method_Silent();
    ASSERT(useAck);

    EV_DEBUG << "MultipleAccessMac::acked(" << frame->getFullName() << ") is ";

    if (lastSentPk && lastSentPk->getTreeId() == frame->getTreeId()) {
        EV_DEBUG << "accepted\n";
        cancelEvent(ackTimeoutMsg);
        delete lastSentPk;
        lastSentPk = nullptr;
        getNextMsgFromHL();
    }
    else
        EV_DEBUG << "unaccepted\n";
}

void MultipleAccessMac::encapsulate(Packet *packet)
{
    auto idealMacHeader = makeShared<IdealMacHeader>();
    idealMacHeader->setChunkLength(B(headerLength));
    auto macAddressReq = packet->getMandatoryTag<MacAddressReq>();
    idealMacHeader->setSrc(macAddressReq->getSrcAddress());
    idealMacHeader->setDest(macAddressReq->getDestAddress());
    MacAddress dest = macAddressReq->getDestAddress();
    if (dest.isBroadcast() || dest.isMulticast() || dest.isUnspecified())
        idealMacHeader->setSrcModuleId(-1);
    else
        idealMacHeader->setSrcModuleId(getId());
    idealMacHeader->setNetworkProtocol(ProtocolGroup::ethertype.getProtocolNumber(packet->getMandatoryTag<PacketProtocolTag>()->getProtocol()));
    packet->insertHeader(idealMacHeader);
}

bool MultipleAccessMac::dropFrameNotForUs(Packet *packet)
{
    auto idealMacHeader = packet->peekHeader<IdealMacHeader>();
    // Current implementation does not support the configuration of multicast
    // MAC address groups. We rather accept all multicast frames (just like they were
    // broadcasts) and pass it up to the higher layer where they will be dropped
    // if not needed.
    // All frames must be passed to the upper layer if the interface is
    // in promiscuous mode.

    if (idealMacHeader->getDest().equals(address))
        return false;

    if (idealMacHeader->getDest().isBroadcast())
        return false;

    if (promiscuous || idealMacHeader->getDest().isMulticast())
        return false;

    EV << "Frame '" << packet->getName() << "' not destined to us, discarding\n";
    PacketDropDetails details;
    details.setReason(NOT_ADDRESSED_TO_US);
    emit(packetDropSignal, packet, &details);
    delete packet;
    return true;
}

void MultipleAccessMac::decapsulate(Packet *packet)
{
    const auto& idealMacHeader = packet->popHeader<IdealMacHeader>();
    auto macAddressInd = packet->ensureTag<MacAddressInd>();
    macAddressInd->setSrcAddress(idealMacHeader->getSrc());
    macAddressInd->setDestAddress(idealMacHeader->getDest());
    packet->ensureTag<InterfaceInd>()->setInterfaceId(interfaceEntry->getInterfaceId());
    packet->ensureTag<DispatchProtocolReq>()->setProtocol(ProtocolGroup::ethertype.getProtocol(idealMacHeader->getNetworkProtocol()));
    packet->ensureTag<PacketProtocolTag>()->setProtocol(ProtocolGroup::ethertype.getProtocol(idealMacHeader->getNetworkProtocol()));
}

} // namespace inet

