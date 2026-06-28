//
// Copyright (C) 2009 - today Brno University of Technology, Czech Republic
// Copyright (C) OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
// Babel routing protocol (RFC 6126) ported from the ANSAINET project.
// Original authors: Vit Rek, Vladimir Vesely (Brno University of Technology).
//

#include "inet/routing/babel/Babel.h"

#include "inet/common/ModuleAccess.h"

namespace inet {
namespace babel {

Define_Module(Babel);

Babel::~Babel()
{
}

void Babel::initialize(int stage)
{
    RoutingProtocolBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        host = getContainingNode(this);
        ift.reference(this, "interfaceTableModule", true);
        udpPort = par("udpPort");
        socket.setOutputGate(gate("socketOut"));
        WATCH(udpPort);
    }
}

void Babel::handleStartOperation(LifecycleOperation *operation)
{
    socket.setMulticastLoop(false);
    socket.bind(udpPort);
    EV_INFO << "Babel routing protocol started, listening on UDP port " << udpPort << "." << endl;
}

void Babel::handleStopOperation(LifecycleOperation *operation)
{
    socket.close();
}

void Babel::handleCrashOperation(LifecycleOperation *operation)
{
    socket.destroy();
}

void Babel::handleMessageWhenUp(cMessage *msg)
{
    if (msg->isSelfMessage()) {
        throw cRuntimeError("Babel: unexpected self message");
    }
    else if (msg->getKind() == UDP_I_DATA) {
        // TODO parse and process the received Babel message (added in a later phase)
        EV_DETAIL << "Babel: received a packet (not yet processed)." << endl;
        delete msg;
    }
    else if (msg->getKind() == UDP_I_ERROR) {
        EV_DETAIL << "Babel: ignoring UDP error report." << endl;
        delete msg;
    }
    else if (msg->getKind() == UDP_I_SOCKET_CLOSED) {
        EV_DETAIL << "Babel: ignoring UDP socket closed indication." << endl;
        delete msg;
    }
    else
        throw cRuntimeError("Babel: unknown message kind (%d)", (int)msg->getKind());
}

} // namespace babel
} // namespace inet
