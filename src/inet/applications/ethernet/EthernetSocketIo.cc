//
// Copyright (C) 2020 OpenSim Ltd.
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

#include "inet/applications/ethernet/EthernetSocketIo.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/socket/SocketTag_m.h"
#include "inet/linklayer/common/MacAddress.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/networklayer/common/FragmentationTag_m.h"
#include "inet/networklayer/common/L3AddressResolver.h"

namespace inet {

Define_Module(EthernetSocketIo);

void EthernetSocketIo::initialize(int stage)
{
    ApplicationBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        const char *localAddressString = par("localAddress");
        if (*localAddressString != '\0')
            localAddress = MacAddress(localAddressString);
        const char *remoteAddressString = par("remoteAddress");
        if (*remoteAddressString != '\0')
            remoteAddress = MacAddress(remoteAddressString);
        numSent = 0;
        numReceived = 0;
        WATCH(numSent);
        WATCH(numReceived);
    }
}

void EthernetSocketIo::handleMessageWhenUp(cMessage *message)
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

void EthernetSocketIo::finish()
{
    recordScalar("packets sent", numSent);
    recordScalar("packets received", numReceived);
    ApplicationBase::finish();
}

void EthernetSocketIo::refreshDisplay() const
{
    ApplicationBase::refreshDisplay();
    char buf[100];
    sprintf(buf, "rcvd: %d pks\nsent: %d pks", numReceived, numSent);
    getDisplayString().setTagArg("t", 0, buf);
}

void EthernetSocketIo::setSocketOptions()
{
    socket.setCallback(this);
    const char *interface = par("interface");
    if (interface[0] != '\0') {
        auto interfaceTable = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
        auto networkInterface = interfaceTable->findInterfaceByName(interface);
        if (networkInterface == nullptr)
            throw cRuntimeError("Cannot find network interface");
        socket.setNetworkInterface(networkInterface);
    }
}

void EthernetSocketIo::socketDataArrived(EthernetSocket *socket, Packet *packet)
{
    emit(packetReceivedSignal, packet);
    EV_INFO << "Received packet: " << packet << endl;
    numReceived++;
    packet->removeTag<SocketInd>();
    send(packet, "trafficOut");
}

void EthernetSocketIo::socketErrorArrived(EthernetSocket *socket, Indication *indication)
{
    EV_WARN << "Ignoring Ethernet error report " << indication->getName() << endl;
    delete indication;
}

void EthernetSocketIo::socketClosed(EthernetSocket *socket)
{
    if (operationalState == State::STOPPING_OPERATION)
        startActiveOperationExtraTimeOrFinish(par("stopOperationExtraTime"));
}

void EthernetSocketIo::handleStartOperation(LifecycleOperation *operation)
{
    setSocketOptions();
    socket.setOutputGate(gate("socketOut"));
// TODO breaks if there are queueing components between the application and the socket handling modules
//    socket.bind(localAddress, remoteAddress, nullptr, true);
}

void EthernetSocketIo::handleStopOperation(LifecycleOperation *operation)
{
    socket.close();
    delayActiveOperationFinish(par("stopOperationTimeout"));
}

void EthernetSocketIo::handleCrashOperation(LifecycleOperation *operation)
{
    socket.destroy();
}

} // namespace inet

