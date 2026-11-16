//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "CoapTestPeer.h"

#include <algorithm>

#include "inet/common/ModuleAccess.h"
#include "inet/common/Simsignals.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/packet/chunk/BytesChunk.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/transportlayer/common/L4PortTag_m.h"

namespace inet {
namespace protocoltest {

Define_Module(CoapTestPeer);

CoapTestPeer::~CoapTestPeer()
{
    cancelAndDelete(sendTimer);
    sendTimer = nullptr;
}

void CoapTestPeer::initialize(int stage)
{
    ApplicationBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        localPort = par("localPort");
        if (sendTimer == nullptr)
            sendTimer = new cMessage("coapTestPeerSendTimer");
    }
}

void CoapTestPeer::handleStartOperation(LifecycleOperation *operation)
{
    const char *localAddrStr = par("localAddress");
    L3Address localAddr = (localAddrStr != nullptr && localAddrStr[0] != '\0')
            ? L3AddressResolver().resolve(localAddrStr)
            : L3Address();

    socket.setOutputGate(gate("socketOut"));
    socket.setCallback(this);
    socket.bind(localAddr, localPort);
    scheduleNextTimer();
}

void CoapTestPeer::handleStopOperation(LifecycleOperation *operation)
{
    cancelEvent(sendTimer);
    socket.close();
    delayActiveOperationFinish(par("stopOperationTimeout"));
}

void CoapTestPeer::handleCrashOperation(LifecycleOperation *operation)
{
    cancelEvent(sendTimer);
    if (operation->getRootModule() != getContainingNode(this))
        socket.destroy();
    else
        socket.setCallback(nullptr);
}

void CoapTestPeer::socketClosed(UdpSocket *closedSocket)
{
    if (operationalState == State::STOPPING_OPERATION)
        startActiveOperationExtraTimeOrFinish(par("stopOperationExtraTime"));
}

void CoapTestPeer::socketErrorArrived(UdpSocket *errSocket, Indication *indication)
{
    delete indication;
}

void CoapTestPeer::scheduleNextTimer()
{
    if (sendTimer == nullptr || operationalState != State::OPERATING || scheduledSends.empty())
        return;

    cancelEvent(sendTimer);
    simtime_t nextTime = scheduledSends.front().time;
    for (const auto& item : scheduledSends) {
        if (item.time < nextTime)
            nextTime = item.time;
    }
    scheduleAt(std::max(nextTime, simTime()), sendTimer);
}

void CoapTestPeer::handleMessageWhenUp(cMessage *msg)
{
    if (msg == sendTimer) {
        simtime_t now = simTime();
        std::deque<ScheduledSend> remaining;
        while (!scheduledSends.empty()) {
            ScheduledSend item = std::move(scheduledSends.front());
            scheduledSends.pop_front();
            if (item.time <= now)
                sendRawNow(item.destAddrStr, item.destPort, item.bytes, item.packetName.c_str());
            else
                remaining.push_back(std::move(item));
        }
        scheduledSends = std::move(remaining);
        scheduleNextTimer();
    }
    else if (socket.belongsToSocket(msg)) {
        socket.processMessage(msg);
    }
    else {
        throw cRuntimeError("Unexpected message at CoapTestPeer: %s", msg->getName());
    }
}

void CoapTestPeer::socketDataArrived(UdpSocket *arrivedSocket, Packet *packet)
{
    if (operationalState != State::OPERATING) {
        delete packet;
        return;
    }

    ReceivedDatagram dgram;
    dgram.arrivalTime = simTime();
    if (const auto& l3Ind = packet->findTag<L3AddressInd>()) {
        dgram.srcAddr = l3Ind->getSrcAddress();
        dgram.destAddr = l3Ind->getDestAddress();
    }
    if (const auto& l4Ind = packet->findTag<L4PortInd>()) {
        dgram.srcPort = l4Ind->getSrcPort();
        dgram.destPort = l4Ind->getDestPort();
    }
    if (packet->getDataLength() > B(0))
        dgram.bytes = packet->peekDataAsBytes()->getBytes();

    receivedDatagrams.push_back(dgram);
    if (inboundCallback)
        inboundCallback(*this, dgram);

    emit(packetReceivedSignal, packet);
    emit(packetReceivedFromLowerSignal, packet);

    delete packet;
}

void CoapTestPeer::sendRawNow(const L3Address& destAddr, int destPort, const std::vector<uint8_t>& bytes,
        const char *packetName)
{
    Enter_Method_Silent();
    Packet *packet = new Packet(packetName);
    if (!bytes.empty())
        packet->insertAtBack(makeShared<BytesChunk>(bytes));
    emit(packetSentSignal, packet);
    emit(packetSentToLowerSignal, packet);
    socket.sendTo(packet, destAddr, destPort);
}

void CoapTestPeer::sendRawNow(const std::string& destAddrStr, int destPort, const std::vector<uint8_t>& bytes,
        const char *packetName)
{
    L3Address destAddr = L3AddressResolver().resolve(destAddrStr.c_str());
    sendRawNow(destAddr, destPort, bytes, packetName);
}

void CoapTestPeer::scheduleRawSend(simtime_t atTime, const std::string& destAddrStr, int destPort,
        const std::vector<uint8_t>& bytes, const char *packetName)
{
    Enter_Method_Silent();
    ScheduledSend item;
    item.time = atTime;
    item.destAddrStr = destAddrStr;
    item.destPort = destPort;
    item.bytes = bytes;
    item.packetName = packetName;
    scheduledSends.push_back(std::move(item));
    scheduleNextTimer();
}

} // namespace protocoltest
} // namespace inet
