//
// Copyright (C) 2011 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/applications/udpapp/UdpEchoApp.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/Simsignals.h"
#include "inet/common/socket/SocketTag_m.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/transportlayer/common/L4PortTag_m.h"
#include "inet/transportlayer/contract/udp/UdpControlInfo_m.h"

namespace inet {

Define_Module(UdpEchoApp);

void UdpEchoApp::initialize(int stage)
{
    ApplicationBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        // init statistics
        numEchoed = 0;
        WATCH(numEchoed);
    }
}

void UdpEchoApp::handleMessageWhenUp(cMessage *msg)
{
    socket.processMessage(msg);
}

void UdpEchoApp::socketDataArrived(UdpSocket *socket, Packet *pk)
{
    // determine its source address/port
    L3Address remoteAddress = pk->getTag<L3AddressInd>()->getSrcAddress();
    int srcPort = pk->getTag<L4PortInd>()->getSrcPort();
    pk->clearTags();
    pk->trim();

    // statistics
    numEchoed++;
    emit(packetSentSignal, pk);
    // send back
    socket->sendTo(pk, remoteAddress, srcPort);
}

void UdpEchoApp::socketErrorArrived(UdpSocket *socket, Indication *indication)
{
    EV_WARN << "Ignoring UDP error report " << indication->getName() << endl;
    delete indication;
}

void UdpEchoApp::socketClosed(UdpSocket *socket)
{
    Enter_Method("socketClosed");
    if (operationalState == State::STOPPING_OPERATION)
        startActiveOperationExtraTimeOrFinish(par("stopOperationExtraTime"));
}

void UdpEchoApp::refreshDisplay() const
{
    ApplicationBase::refreshDisplay();

    char buf[40];
    sprintf(buf, "echoed: %d pks", numEchoed);
    getDisplayString().setTagArg("t", 0, buf);
}

void UdpEchoApp::finish()
{
    ApplicationBase::finish();
}

void UdpEchoApp::handleStartOperation(LifecycleOperation *operation)
{
    socket.setOutputGate(gate("socketOut"));
    int localPort = par("localPort");
    socket.bind(localPort);
    MulticastGroupList mgl = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this)->collectMulticastGroups();
    socket.joinLocalMulticastGroups(mgl);
    socket.setCallback(this);
}

void UdpEchoApp::handleStopOperation(LifecycleOperation *operation)
{
    socket.close();
    delayActiveOperationFinish(par("stopOperationTimeout"));
}

void UdpEchoApp::handleCrashOperation(LifecycleOperation *operation)
{
    if (operation->getRootModule() != getContainingNode(this)) // closes socket when the application crashed only
        socket.destroy(); // TODO  in real operating systems, program crash detected by OS and OS closes sockets of crashed programs.
    socket.setCallback(nullptr);
}

void UdpEchoApp::pushPacket(Packet *packet, const cGate *gate)
{
    Enter_Method("pushPacket");
    take(packet);
    socket.processMessage(packet);
}

cGate *UdpEchoApp::lookupModuleInterface(cGate *gate, const std::type_info& type, const cObject *arguments, int direction)
{
    Enter_Method("lookupModuleInterface");
    EV_TRACE << "Looking up module interface" << EV_FIELD(gate) << EV_FIELD(type, opp_typename(type)) << EV_FIELD(arguments) << EV_FIELD(direction) << EV_ENDL;
    if (gate->isName("socketIn")) {
        if (type == typeid(IPassivePacketSink)) {
            auto socketInd = dynamic_cast<const SocketInd *>(arguments);
            if (socketInd != nullptr && socketInd->getSocketId() == socket.getSocketId())
                return gate;
        }
    }
    return nullptr;
}

} // namespace inet

