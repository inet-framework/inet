//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/applications/ipapp/IpSocketIo.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/socket/SocketTag_m.h"
#include "inet/networklayer/common/FragmentationTag_m.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/common/SimulationContinuation.h"

namespace inet {

Define_Module(IpSocketIo);

void IpSocketIo::initialize(int stage)
{
    ApplicationBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        const char *protocolAsString = par("protocol");
        if (!opp_isempty(protocolAsString))
            protocol = Protocol::getProtocol(protocolAsString);
        trafficOutSink.reference(gate("trafficOut"), true);
        numSent = 0;
        numReceived = 0;
        WATCH(dontFragment);
        WATCH(numSent);
        WATCH(numReceived);
    }
}

void IpSocketIo::handleMessageWhenUp(cMessage *message)
{
    if (socket.belongsToSocket(message))
        socket.processMessage(message);
    else {
        auto packet = check_and_cast<Packet *>(message);
        if (dontFragment)
            packet->addTagIfAbsent<FragmentationReq>()->setDontFragment(true);
        socket.send(packet);
        numSent++;
        emit(packetSentSignal, packet);
    }
}

void IpSocketIo::finish()
{
    recordScalar("packets sent", numSent);
    recordScalar("packets received", numReceived);
    ApplicationBase::finish();
}

void IpSocketIo::setSocketOptions()
{
    socket.setCallback(this);
}

void IpSocketIo::socketDataArrived(Ipv4Socket *socket, Packet *packet)
{
    Enter_Method("socketDataArrived");
    emit(packetReceivedSignal, packet);
    numReceived++;
    packet->removeTag<SocketInd>();
    yieldBeforePush();
    trafficOutSink.pushPacket(packet);
}

void IpSocketIo::socketClosed(Ipv4Socket *socket)
{
    Enter_Method("socketClosed");
    if (operationalState == State::STOPPING_OPERATION)
        startActiveOperationExtraTimeOrFinish(par("stopOperationExtraTime"));
}

void IpSocketIo::handleStartOperation(LifecycleOperation *operation)
{
    socket.setOutputGate(gate("socketOut"));
    setSocketOptions();
    const char *localAddress = par("localAddress");
    socket.bind(protocol, *localAddress ? L3AddressResolver().resolve(localAddress).toIpv4() : Ipv4Address());
    const char *destAddrs = par("destAddress");
    if (*destAddrs)
        socket.connect(L3AddressResolver().resolve(destAddrs).toIpv4());
}

void IpSocketIo::handleStopOperation(LifecycleOperation *operation)
{
    // register the delayed finish before close(): the closed callback may complete the operation synchronously
    delayActiveOperationFinish(par("stopOperationTimeout"));
    socket.close();
}

void IpSocketIo::handleCrashOperation(LifecycleOperation *operation)
{
    socket.destroy();
}

void IpSocketIo::pushPacket(Packet *packet, const cGate *gate)
{
    Enter_Method("pushPacket");
    take(packet);
    handleMessage(packet);
}

} // namespace inet

