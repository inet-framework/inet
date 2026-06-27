//
// Copyright (C) 2013 - today Brno University of Technology, Czech Republic
// Copyright (C) OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
// LISP (Locator/ID Separation Protocol, RFC 6830) ported from the ANSAINET project.
// Original author: Vladimir Vesely (Brno University of Technology).
//

#include "inet/routing/lisp/Lisp.h"

#include "inet/common/ModuleAccess.h"
#include "inet/networklayer/common/L3AddressResolver.h"

namespace inet {
namespace lisp {

Define_Module(Lisp);

void Lisp::initialize(int stage)
{
    ApplicationBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        controlPort = par("controlPort");
        dataPort = par("dataPort");
    }
}

void Lisp::handleStartOperation(LifecycleOperation *operation)
{
    controlSocket.setOutputGate(gate("socketOut"));
    controlSocket.setCallback(this);
    controlSocket.bind(L3Address(), controlPort);
    socketMap.addSocket(&controlSocket);

    dataSocket.setOutputGate(gate("socketOut"));
    dataSocket.setCallback(this);
    dataSocket.bind(L3Address(), dataPort);
    socketMap.addSocket(&dataSocket);
}

void Lisp::handleMessageWhenUp(cMessage *msg)
{
    if (msg->isSelfMessage())
        throw cRuntimeError("Unexpected self message '%s'", msg->getName());
    else if (ISocket *socket = socketMap.findSocketFor(msg))
        socket->processMessage(msg);
    else
        throw cRuntimeError("Unknown message '%s'", msg->getName());
}

void Lisp::socketDataArrived(UdpSocket *socket, Packet *packet)
{
    // TODO dispatch control (4342) vs data (4341) messages (added in later steps)
    EV_INFO << "LISP received " << UdpSocket::getReceivedPacketInfo(packet)
            << " on " << (socket == &controlSocket ? "control" : "data") << " socket\n";
    delete packet;
}

void Lisp::socketErrorArrived(UdpSocket *socket, Indication *indication)
{
    EV_WARN << "LISP UDP error: " << indication->getName() << "\n";
    delete indication;
}

void Lisp::socketClosed(UdpSocket *socket)
{
    // TODO proper async stop completion (added together with the lifecycle work)
}

void Lisp::handleStopOperation(LifecycleOperation *operation)
{
    controlSocket.close();
    dataSocket.close();
}

void Lisp::handleCrashOperation(LifecycleOperation *operation)
{
    controlSocket.destroy();
    dataSocket.destroy();
}

} // namespace lisp
} // namespace inet
