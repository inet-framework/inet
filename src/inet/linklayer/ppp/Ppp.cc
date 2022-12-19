//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ppp/Ppp.h"

#include <stdio.h>
#include <string.h>

#include "inet/common/INETUtils.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolGroup.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/Simsignals.h"
#include "inet/common/StringFormat.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/networklayer/contract/IInterfaceTable.h"

namespace inet {

Define_Module(Ppp);

simsignal_t Ppp::transmissionStateChangedSignal = registerSignal("transmissionStateChanged");
simsignal_t Ppp::rxPkOkSignal = registerSignal("rxPkOk");

Ppp::~Ppp()
{
    cancelAndDelete(endTransmissionEvent);
    delete curTxPacket;
}

void Ppp::initialize(int stage)
{
    MacProtocolBase::initialize(stage);

    // all initialization is done in the first stage
    if (stage == INITSTAGE_LOCAL) {
        displayStringTextFormat = par("displayStringTextFormat");
        sendRawBytes = par("sendRawBytes");
        endTransmissionEvent = new cMessage("pppEndTxEvent");
        lowerLayerInGateId = findGate("phys$i");
        physOutGate = gate("phys$o");
        lowerLayerOutGateId = physOutGate->getId();

        setTxUpdateSupport(true);
        // we're connected if other end of connection path is an input gate
        bool connected = physOutGate->getPathEndGate()->getType() == cGate::INPUT;
        // if we're connected, get the gate with transmission rate
        datarateChannel = connected ? physOutGate->getTransmissionChannel() : nullptr;

        numSent = numRcvdOK = numDroppedBitErr = numDroppedIfaceDown = 0;
        WATCH(numSent);
        WATCH(numRcvdOK);
        WATCH(numDroppedBitErr);
        WATCH(numDroppedIfaceDown);

        subscribe(PRE_MODEL_CHANGE, this);
        subscribe(POST_MODEL_CHANGE, this);
        emit(transmissionStateChangedSignal, 0L);

        txQueue = getQueue(gate(upperLayerInGateId));
    }
}

void Ppp::configureNetworkInterface()
{
    // data rate
    bool connected = datarateChannel != nullptr;
    double datarate = connected ? datarateChannel->getNominalDatarate() : 0;
    networkInterface->setDatarate(datarate);
    networkInterface->setCarrier(connected);

    // generate a link-layer address to be used as interface token for IPv6
    InterfaceToken token(0, getActiveSimulationOrEnvir()->getUniqueNumber(), 64);
    networkInterface->setInterfaceToken(token);

    // MTU: typical values are 576 (Internet de facto), 1500 (Ethernet-friendly),
    // 4000 (on some point-to-point links), 4470 (Cisco routers default, FDDI compatible)
    networkInterface->setMtu(par("mtu"));

    // capabilities
    networkInterface->setMulticast(true);
    networkInterface->setPointToPoint(true);
}

void Ppp::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details)
{
    Enter_Method("%s", cComponent::getSignalName(signalID));

    MacProtocolBase::receiveSignal(source, signalID, obj, details);

#if OMNETPP_BUILDNUM < 2001
    if (getSimulation()->getSimulationStage() == STAGE(CLEANUP))
#else
    if (getSimulation()->getStage() == STAGE(CLEANUP))
#endif
        return;

    if (signalID == POST_MODEL_CHANGE) {
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
    else if (signalID == PRE_MODEL_CHANGE) {
        if (auto gcobj = dynamic_cast<cPrePathCutNotification *>(obj)) {
            if (physOutGate == gcobj->pathStartGate)
                refreshOutGateConnection(false);
        }
    }
}

void Ppp::refreshOutGateConnection(bool connected)
{
    Enter_Method("refreshOutGateConnection");

    // we're connected if other end of connection path is an input gate
    if (connected)
        ASSERT(physOutGate->getPathEndGate()->getType() == cGate::INPUT);

    if (!connected) {
        if (endTransmissionEvent->isScheduled()) {
            ASSERT(curTxPacket != nullptr);
            simtime_t startTransmissionTime = endTransmissionEvent->getSendingTime();
            simtime_t sentDuration = simTime() - startTransmissionTime;
            double sentPart = sentDuration / (endTransmissionEvent->getArrivalTime() - startTransmissionTime);
            b newLength = b(floor(curTxPacket->getBitLength() * sentPart));
            curTxPacket->removeAtBack(curTxPacket->getDataLength() - newLength);
            curTxPacket->setBitError(true);
            send(curTxPacket, SendOptions().finishTx(curTxPacket->getId()), physOutGate);
            curTxPacket = nullptr;
            cancelEvent(endTransmissionEvent);
        }

        PacketDropDetails details;
        details.setReason(INTERFACE_DOWN); // TODO choose a correct PacketDropReason value
        flushQueue(details);
    }

    cChannel *oldChannel = datarateChannel;
    // if we're connected, get the gate with transmission rate
    datarateChannel = connected ? physOutGate->getTransmissionChannel() : nullptr;
    double datarate = connected ? datarateChannel->getNominalDatarate() : 0;

    if (datarateChannel && !oldChannel)
        datarateChannel->subscribe(POST_MODEL_CHANGE, this);

    // update interface state if it is in use
    if (networkInterface) {
        networkInterface->setCarrier(connected);
        networkInterface->setDatarate(datarate);
    }

    if (connected && currentTxFrame == nullptr && canDequeuePacket()) {
        ASSERT(!endTransmissionEvent->isScheduled());
        processUpperPacket();
    }
}

void Ppp::startTransmitting()
{
    // if there's any control info, remove it; then encapsulate the packet
    Packet *pppFrame = currentTxFrame->dup();
    encapsulate(pppFrame);

    // send
    EV_INFO << "Transmission of " << pppFrame << " started.\n";
    emit(transmissionStateChangedSignal, 1L);
    emit(packetSentToLowerSignal, pppFrame);
    auto& oldPacketProtocolTag = pppFrame->removeTag<PacketProtocolTag>();
    pppFrame->clearTags();
    auto newPacketProtocolTag = pppFrame->addTag<PacketProtocolTag>();
    *newPacketProtocolTag = *oldPacketProtocolTag;
    if (sendRawBytes) {
        auto bytes = pppFrame->peekDataAsBytes();
        pppFrame->eraseAll();
        pppFrame->insertAtFront(bytes);
    }
    curTxPacket = pppFrame->dup();
    send(pppFrame, SendOptions().transmissionId(curTxPacket->getId()), physOutGate);

    ASSERT(datarateChannel == physOutGate->getTransmissionChannel()); // FIXME reread datarateChannel when changed

    // schedule an event for the time when last bit will leave the gate.
    simtime_t endTransmissionTime = datarateChannel->getTransmissionFinishTime();
    scheduleAt(endTransmissionTime, endTransmissionEvent);
    numSent++;
}

void Ppp::handleMessageWhenUp(cMessage *message)
{
    MacProtocolBase::handleMessageWhenUp(message);
    if (operationalState == State::STOPPING_OPERATION) {
        if (txQueue->isEmpty()) {
            networkInterface->setCarrier(false);
            networkInterface->setState(NetworkInterface::State::DOWN);
            startActiveOperationExtraTimeOrFinish(par("stopOperationExtraTime"));
        }
    }
}

void Ppp::handleSelfMessage(cMessage *message)
{
    if (message == endTransmissionEvent) {
        deleteCurrentTxFrame();
        delete curTxPacket;
        curTxPacket = nullptr;
        // Transmission finished, we can start next one.
        EV_INFO << "Transmission successfully completed.\n";
        emit(transmissionStateChangedSignal, 0L);
        if (canDequeuePacket())
            processUpperPacket();
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
    if (currentTxFrame != nullptr)
        throw cRuntimeError("PPP already in transmit state when packet arrived from upper layer");
    currentTxFrame = packet;
    startTransmitting();
}

void Ppp::handleLowerPacket(Packet *packet)
{
    // TODO if incoming gate is not connected now, then the link has been deleted
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
        decapsulate(packet);
        numRcvdOK++;
        emit(packetSentToUpperSignal, packet);
        EV_INFO << "Sending " << packet << " to upper layer.\n";
        send(packet, "upperLayerOut");
    }
}

void Ppp::refreshDisplay() const
{
    MacProtocolBase::refreshDisplay();

    if (displayStringTextFormat != nullptr) {
        auto text = StringFormat::formatString(displayStringTextFormat, [&] (char directive) -> std::string {
            switch (directive) {
                case 's':
                    return std::to_string(numSent);
                case 'r':
                    return std::to_string(numRcvdOK);
                case 'd':
                    return std::to_string(numDroppedIfaceDown + numDroppedBitErr);
                case 'q':
                    return std::to_string(txQueue->getNumPackets());
                case 'b':
                    if (datarateChannel == nullptr)
                        return "not connected";
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
                        return datarateText;
                    }
                default:
                    throw cRuntimeError("Unknown directive: %c", directive);
            }
        });
        getDisplayString().setTagArg("t", 0, text.c_str());
    }

    const char *color = "";
    if (datarateChannel != nullptr) {
        if (endTransmissionEvent->isScheduled())
            color = txQueue->getNumPackets() >= 3 ? "red" : "yellow";
    }
    else
        color = "#707070";
    getDisplayString().setTagArg("i", 1, color);
}

void Ppp::encapsulate(Packet *packet)
{
    auto pppHeader = makeShared<PppHeader>();
    pppHeader->setProtocol(ProtocolGroup::getPppProtocolGroup()->getProtocolNumber(packet->getTag<PacketProtocolTag>()->getProtocol()));
    packet->insertAtFront(pppHeader);
    auto pppTrailer = makeShared<PppTrailer>();
    packet->insertAtBack(pppTrailer);
    packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::ppp);
}

void Ppp::decapsulate(Packet *packet)
{
    const auto& pppHeader = packet->popAtFront<PppHeader>();
    const auto& pppTrailer = packet->popAtBack<PppTrailer>(PPP_TRAILER_LENGTH);
    if (pppHeader == nullptr || pppTrailer == nullptr)
        throw cRuntimeError("Invalid PPP packet: PPP header or Trailer is missing");
    // TODO check CRC
    packet->addTagIfAbsent<InterfaceInd>()->setInterfaceId(networkInterface->getInterfaceId());

    auto payloadProtocol = ProtocolGroup::getPppProtocolGroup()->getProtocol(pppHeader->getProtocol());
    packet->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(payloadProtocol);
    packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(payloadProtocol);
}

void Ppp::handleStopOperation(LifecycleOperation *operation)
{
    if (!txQueue->isEmpty()) {
        networkInterface->setState(NetworkInterface::State::GOING_DOWN);
        delayActiveOperationFinish(par("stopOperationTimeout"));
    }
    else {
        networkInterface->setCarrier(false);
        networkInterface->setState(NetworkInterface::State::DOWN);
        startActiveOperationExtraTimeOrFinish(par("stopOperationExtraTime"));
    }
}

queueing::IPassivePacketSource *Ppp::getProvider(cGate *gate)
{
    return (gate->getId() == upperLayerInGateId) ? txQueue.get() : nullptr;
}

void Ppp::handleCanPullPacketChanged(cGate *gate)
{
    Enter_Method("handleCanPullPacketChanged");
    while (currentTxFrame == nullptr && canDequeuePacket())
        processUpperPacket();
}

void Ppp::handlePullPacketProcessed(Packet *packet, cGate *gate, bool successful)
{
    Enter_Method("handlePullPacketProcessed");
    throw cRuntimeError("Not supported callback");
}

void Ppp::processUpperPacket()
{
    auto packet = dequeuePacket();
    handleUpperPacket(packet);
}

} // namespace inet

