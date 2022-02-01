//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/applications/ieee8022/Ieee8022LlcSocketIo.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/socket/SocketTag_m.h"
#include "inet/linklayer/common/MacAddress.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/networklayer/common/FragmentationTag_m.h"
#include "inet/networklayer/common/L3AddressResolver.h"

namespace inet {

Define_Module(Ieee8022LlcSocketIo);

void Ieee8022LlcSocketIo::initialize(int stage)
{
    ApplicationBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        localSap = par("localSap");
        remoteSap = par("remoteSap");
        numSent = 0;
        numReceived = 0;
        WATCH(numSent);
        WATCH(numReceived);
    }
    else if (stage == INITSTAGE_NETWORK_INTERFACE_CONFIGURATION + 1) {
        const char *localAddressString = par("localAddress");
        if (*localAddressString != '\0') {
            L3Address l3Address;
            L3AddressResolver addressResolver;
            addressResolver.tryResolve(localAddressString, l3Address, L3AddressResolver::ADDR_MAC);
            if (l3Address.getType() == L3Address::MAC)
                localAddress = l3Address.toMac();
            else
                localAddress = MacAddress(localAddressString);
        }
        const char *remoteAddressString = par("remoteAddress");
        if (*remoteAddressString != '\0') {
            L3Address l3Address;
            L3AddressResolver addressResolver;
            addressResolver.tryResolve(remoteAddressString, l3Address, L3AddressResolver::ADDR_MAC);
            if (l3Address.getType() == L3Address::MAC)
                remoteAddress = l3Address.toMac();
            else
                remoteAddress = MacAddress(remoteAddressString);
        }
        const char *interface = par("interface");
        if (interface[0] != '\0') {
            auto interfaceTable = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
            networkInterface = interfaceTable->findInterfaceByName(interface);
            if (networkInterface == nullptr)
                throw cRuntimeError("Cannot find network interface");
        }
    }
}

void Ieee8022LlcSocketIo::handleMessageWhenUp(cMessage *message)
{
    if (socket.belongsToSocket(message))
        socket.processMessage(message);
    else {
        auto packet = check_and_cast<Packet *>(message);
        if (packet->findTag<PacketProtocolTag>() == nullptr) {
            auto packetProtocolTag = packet->addTag<PacketProtocolTag>();
            packetProtocolTag->setProtocol(&Protocol::unknown);
        }
        auto& macAddressReq = packet->addTag<MacAddressReq>();
        macAddressReq->setDestAddress(remoteAddress);
        socket.send(packet);
        numSent++;
        emit(packetSentSignal, packet);
    }
}

void Ieee8022LlcSocketIo::finish()
{
    recordScalar("packets sent", numSent);
    recordScalar("packets received", numReceived);
    ApplicationBase::finish();
}

void Ieee8022LlcSocketIo::refreshDisplay() const
{
    ApplicationBase::refreshDisplay();
    char buf[100];
    sprintf(buf, "rcvd: %d pks\nsent: %d pks", numReceived, numSent);
    getDisplayString().setTagArg("t", 0, buf);
}

void Ieee8022LlcSocketIo::setSocketOptions()
{
    socket.setCallback(this);
}

void Ieee8022LlcSocketIo::socketDataArrived(Ieee8022LlcSocket *socket, Packet *packet)
{
    emit(packetReceivedSignal, packet);
    EV_INFO << "Received packet: " << packet << endl;
    numReceived++;
    packet->removeTag<SocketInd>();
    send(packet, "trafficOut");
}

void Ieee8022LlcSocketIo::socketClosed(Ieee8022LlcSocket *socket)
{
    if (operationalState == State::STOPPING_OPERATION)
        startActiveOperationExtraTimeOrFinish(par("stopOperationExtraTime"));
}

void Ieee8022LlcSocketIo::handleStartOperation(LifecycleOperation *operation)
{
    setSocketOptions();
    socket.setOutputGate(gate("socketOut"));
    socket.open(networkInterface != nullptr ? networkInterface->getInterfaceId() : -1, localSap, remoteSap);
}

void Ieee8022LlcSocketIo::handleStopOperation(LifecycleOperation *operation)
{
    socket.close();
    delayActiveOperationFinish(par("stopOperationTimeout"));
}

void Ieee8022LlcSocketIo::handleCrashOperation(LifecycleOperation *operation)
{
    socket.destroy();
}

} // namespace inet

