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

namespace inet {

Define_Module(IpSocketIo);

void IpSocketIo::initialize(int stage)
{
    ApplicationBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        const char *protocolAsString = par("protocol");
        if (!opp_isempty(protocolAsString))
            protocol = Protocol::getProtocol(protocolAsString);
        numSent = 0;
        numReceived = 0;
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

void IpSocketIo::refreshDisplay() const
{
    ApplicationBase::refreshDisplay();
    char buf[100];
    sprintf(buf, "rcvd: %d pks\nsent: %d pks", numReceived, numSent);
    getDisplayString().setTagArg("t", 0, buf);
}

void IpSocketIo::setSocketOptions()
{
    socket.setCallback(this);
}

void IpSocketIo::socketDataArrived(Ipv4Socket *socket, Packet *packet)
{
    emit(packetReceivedSignal, packet);
    numReceived++;
    packet->removeTag<SocketInd>();
    send(packet, "trafficOut");
}

void IpSocketIo::socketClosed(Ipv4Socket *socket)
{
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
    socket.close();
    delayActiveOperationFinish(par("stopOperationTimeout"));
}

void IpSocketIo::handleCrashOperation(LifecycleOperation *operation)
{
    socket.destroy();
}

} // namespace inet

