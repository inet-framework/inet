//
// Copyright (C) 2004 Andras Varga
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

#include <stdio.h>
#include <string.h>

#include "inet/common/INETDefs.h"
#include "inet/common/INETUtils.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolGroup.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/Simsignals.h"
#include "inet/common/StringFormat.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/ppp/Ppp.h"
#include "inet/networklayer/contract/IInterfaceTable.h"

namespace inet {

Define_Module(Ppp);

simsignal_t Ppp::transmissionStateChangedSignal = registerSignal("transmissionStateChanged");
simsignal_t Ppp::rxPkOkSignal = registerSignal("rxPkOk");

Ppp::~Ppp()
{
    cancelAndDelete(endTransmissionEvent);
}

void Ppp::initialize(int stage)
{
    MacBase::initialize(stage);

    // all initialization is done in the first stage
    if (stage == INITSTAGE_LOCAL) {
        displayStringTextFormat = par("displayStringTextFormat");
        sendRawBytes = par("sendRawBytes");
        endTransmissionEvent = new cMessage("pppEndTxEvent");
        physOutGate = gate("phys$o");
        // we're connected if other end of connection path is an input gate
        bool connected = physOutGate->getPathEndGate()->getType() == cGate::INPUT;
        // if we're connected, get the gate with transmission rate
        datarateChannel = connected ? physOutGate->getTransmissionChannel() : nullptr;

        numSent = numRcvdOK = numDroppedBitErr = numDroppedIfaceDown = 0;
        WATCH(numSent);
        WATCH(numRcvdOK);
        WATCH(numDroppedBitErr);
        WATCH(numDroppedIfaceDown);

        subscribe(POST_MODEL_CHANGE, this);
        emit(transmissionStateChangedSignal, 0L);

        queue = check_and_cast<queueing::IPacketQueue *>(getSubmodule("queue"));
    }
}

void Ppp::configureInterfaceEntry()
{
    // data rate
    bool connected = datarateChannel != nullptr;
    double datarate = connected ? datarateChannel->getNominalDatarate() : 0;
    interfaceEntry->setDatarate(datarate);
    interfaceEntry->setCarrier(connected);

    // generate a link-layer address to be used as interface token for IPv6
    InterfaceToken token(0, getSimulation()->getUniqueNumber(), 64);
    interfaceEntry->setInterfaceToken(token);

    // MTU: typical values are 576 (Internet de facto), 1500 (Ethernet-friendly),
    // 4000 (on some point-to-point links), 4470 (Cisco routers default, FDDI compatible)
    interfaceEntry->setMtu(par("mtu"));

    // capabilities
    interfaceEntry->setMulticast(true);
    interfaceEntry->setPointToPoint(true);
}

void Ppp::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details)
{
    MacBase::receiveSignal(source, signalID, obj, details);

    if (signalID != POST_MODEL_CHANGE)
        return;

    if (auto gcobj = dynamic_cast<cPostPathCreateNotification *>(obj)) {
        if (physOutGate == gcobj->pathStartGate)
            refreshOutGateConnection(true);
    }
    else if (auto gcobj = dynamic_cast<cPostPathCutNotification *>(obj)) {
        if (physOutGate == gcobj->pathStartGate)
            refreshOutGateConnection(false);
    }
    else if (datarateChannel && dynamic_cast<cPostParameterChangeNotification *>(obj)) {
        cPostParameterChangeNotification *gcobj = static_cast<cPostParameterChangeNotification *>(obj);
        if (datarateChannel == gcobj->par->getOwner() && !strcmp("datarate", gcobj->par->getName()))
            refreshOutGateConnection(true);
    }
}

void Ppp::refreshOutGateConnection(bool connected)
{
    Enter_Method_Silent();

    // we're connected if other end of connection path is an input gate
    if (connected)
        ASSERT(physOutGate->getPathEndGate()->getType() == cGate::INPUT);

    if (!connected) {
        if (endTransmissionEvent->isScheduled()) {
            cancelEvent(endTransmissionEvent);

            if (datarateChannel)
                datarateChannel->forceTransmissionFinishTime(SIMTIME_ZERO);
        }

        flushQueue();
    }

    cChannel *oldChannel = datarateChannel;
    // if we're connected, get the gate with transmission rate
    datarateChannel = connected ? physOutGate->getTransmissionChannel() : nullptr;
    double datarate = connected ? datarateChannel->getNominalDatarate() : 0;

    if (datarateChannel && !oldChannel)
        datarateChannel->subscribe(POST_MODEL_CHANGE, this);

    // update interface state if it is in use
    if (interfaceEntry) {
        interfaceEntry->setCarrier(connected);
        interfaceEntry->setDatarate(datarate);
    }

    if (connected && !endTransmissionEvent->isScheduled() && !queue->isEmpty())
        startTransmitting(queue->popPacket());
}

void Ppp::startTransmitting(Packet *msg)
{
    // if there's any control info, remove it; then encapsulate the packet
    Packet *pppFrame = encapsulate(msg);

    // send
    EV_INFO << "Transmission of " << pppFrame << " started.\n";
    emit(transmissionStateChangedSignal, 1L);
    emit(packetSentToLowerSignal, pppFrame);
    auto oldPacketProtocolTag = pppFrame->removeTag<PacketProtocolTag>();
    pppFrame->clearTags();
    auto newPacketProtocolTag = pppFrame->addTag<PacketProtocolTag>();
    *newPacketProtocolTag = *oldPacketProtocolTag;
    delete oldPacketProtocolTag;
    if (sendRawBytes) {
        auto rawFrame = new Packet(pppFrame->getName(), pppFrame->peekAllAsBytes());
        rawFrame->copyTags(*pppFrame);
        send(rawFrame, physOutGate);
        delete pppFrame;
    }
    else
        send(pppFrame, physOutGate);

    ASSERT(datarateChannel == physOutGate->getTransmissionChannel());    //FIXME reread datarateChannel when changed

    // schedule an event for the time when last bit will leave the gate.
    simtime_t endTransmissionTime = datarateChannel->getTransmissionFinishTime();
    scheduleAt(endTransmissionTime, endTransmissionEvent);
    numSent++;
}

void Ppp::handleMessageWhenUp(cMessage *message)
{
    MacBase::handleMessageWhenUp(message);
    if (operationalState == State::STOPPING_OPERATION) {
        if (queue->isEmpty()) {
            interfaceEntry->setCarrier(false);
            interfaceEntry->setState(InterfaceEntry::State::DOWN);
            startActiveOperationExtraTimeOrFinish(par("stopOperationExtraTime"));
        }
    }
}

void Ppp::handleSelfMessage(cMessage *message)
{
    if (message == endTransmissionEvent) {
        // Transmission finished, we can start next one.
        EV_INFO << "Transmission successfully completed.\n";
        emit(transmissionStateChangedSignal, 0L);
        if (!queue->isEmpty())
            startTransmitting(queue->popPacket());
    }
    else
        throw cRuntimeError("Unknown self message");
}

void Ppp::handleUpperPacket(Packet *packet)
{
    EV_INFO << "Received " << packet << " from upper layer.\n";
    if (datarateChannel == nullptr) {
        EV_WARN << "Interface is not connected, dropping packet " << packet << endl;
        numDroppedIfaceDown++;
        PacketDropDetails details;
        details.setReason(INTERFACE_DOWN);
        emit(packetDroppedSignal, packet, &details);
        delete packet;
        return;
    }
    queue->pushPacket(packet);
    if (!endTransmissionEvent->isScheduled() && !queue->isEmpty())
        startTransmitting(queue->popPacket());
}

void Ppp::handleLowerPacket(Packet *packet)
{
    //TODO: if incoming gate is not connected now, then the link has been deleted
    // during packet transmission --> discard incomplete packet.
    EV_INFO << "Received " << packet << " from network.\n";
    packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::ppp);

    // check for bit errors
    if (packet->hasBitError()) {
        EV_WARN << "Bit error in " << packet << endl;
        PacketDropDetails details;
        details.setReason(INCORRECTLY_RECEIVED);
        emit(packetDroppedSignal, packet, &details);
        numDroppedBitErr++;
        delete packet;
    }
    else {
        // pass up payload
        const auto& pppHeader = packet->peekAtFront<PppHeader>();
        const auto& pppTrailer = packet->peekAtBack<PppTrailer>(PPP_TRAILER_LENGTH);
        if (pppHeader == nullptr || pppTrailer == nullptr)
            throw cRuntimeError("Invalid PPP packet: PPP header or Trailer is missing");
        emit(rxPkOkSignal, packet);
        cPacket *payload = decapsulate(packet);
        numRcvdOK++;
        emit(packetSentToUpperSignal, payload);
        EV_INFO << "Sending " << payload << " to upper layer.\n";
        send(payload, "upperLayerOut");
    }
}

void Ppp::refreshDisplay() const
{
    MacBase::refreshDisplay();

    auto text = StringFormat::formatString(displayStringTextFormat, [&] (char directive) {
        static std::string result;
        switch (directive) {
            case 's':
                result = std::to_string(numSent);
                break;
            case 'r':
                result = std::to_string(numRcvdOK);
                break;
            case 'd':
                result = std::to_string(numDroppedIfaceDown + numDroppedBitErr);
                break;
            case 'q':
                result = std::to_string(queue->getNumPackets());
                break;
            case 'b':
                if (datarateChannel == nullptr)
                    result = "not connected";
                else {
                    char datarateText[40];
                    double datarate = datarateChannel->getNominalDatarate();
                    if (datarate >= 1e9)
                        sprintf(datarateText, "%gGbps", datarate / 1e9);
                    else if (datarate >= 1e6)
                        sprintf(datarateText, "%gMbps", datarate / 1e6);
                    else if (datarate >= 1e3)
                        sprintf(datarateText, "%gkbps", datarate / 1e3);
                    else
                        sprintf(datarateText, "%gbps", datarate);
                    result = datarateText;
                }
                break;
            default:
                throw cRuntimeError("Unknown directive: %c", directive);
        }
        return result.c_str();
    });
    getDisplayString().setTagArg("t", 0, text);

    const char *color = "";
    if (datarateChannel != nullptr) {
        if (endTransmissionEvent->isScheduled())
            color = queue->getNumPackets() >= 3 ? "red" : "yellow";
    }
    else
        color = "#707070";
    getDisplayString().setTagArg("i", 1, color);
}

Packet *Ppp::encapsulate(Packet *msg)
{
    auto packet = check_and_cast<Packet*>(msg);
    auto pppHeader = makeShared<PppHeader>();
    pppHeader->setProtocol(ProtocolGroup::pppprotocol.getProtocolNumber(msg->getTag<PacketProtocolTag>()->getProtocol()));
    packet->insertAtFront(pppHeader);
    auto pppTrailer = makeShared<PppTrailer>();
    packet->insertAtBack(pppTrailer);
    packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::ppp);
    return packet;
}

cPacket *Ppp::decapsulate(Packet *packet)
{
    const auto& pppHeader = packet->popAtFront<PppHeader>();
    const auto& pppTrailer = packet->popAtBack<PppTrailer>(PPP_TRAILER_LENGTH);
    if (pppHeader == nullptr || pppTrailer == nullptr)
        throw cRuntimeError("Invalid PPP packet: PPP header or Trailer is missing");
    //TODO check CRC
    packet->addTagIfAbsent<InterfaceInd>()->setInterfaceId(interfaceEntry->getInterfaceId());

    auto payloadProtocol = ProtocolGroup::pppprotocol.getProtocol(pppHeader->getProtocol());
    packet->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(payloadProtocol);
    packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(payloadProtocol);
    return packet;
}

void Ppp::flushQueue()
{
    // code would look slightly nicer with a pop() function that returns nullptr if empty
    while (!queue->isEmpty()) {
        auto packet = queue->popPacket();
        PacketDropDetails details;
        details.setReason(INTERFACE_DOWN);
        emit(packetDroppedSignal, packet, &details); //FIXME this signal lumps together packets from the network and packets from higher layers! separate them
        delete packet;
    }
}

void Ppp::clearQueue()
{
    while (!queue->isEmpty())
        delete queue->popPacket();
}

void Ppp::handleStopOperation(LifecycleOperation *operation)
{
    if (!queue->isEmpty()) {
        interfaceEntry->setState(InterfaceEntry::State::GOING_DOWN);
        delayActiveOperationFinish(par("stopOperationTimeout"));
    }
    else {
        interfaceEntry->setCarrier(false);
        interfaceEntry->setState(InterfaceEntry::State::DOWN);
        startActiveOperationExtraTimeOrFinish(par("stopOperationExtraTime"));
    }
}

} // namespace inet
