//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
// This implementation is based on the IS-IS model of the ANSA project
// (https://ansa.omnetpp.org), Brno University of Technology, ported to the
// modern INET packet (Chunk) API.
//

#include "inet/routing/isis/Isis.h"

#include "inet/common/ModuleAccess.h"

namespace inet {
namespace isis {

Define_Module(Isis);

void Isis::initialize(int stage)
{
    RoutingProtocolBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        ift.reference(this, "interfaceTableModule", true);
        llcSocket.setOutputGate(gate("socketOut"));
        llcSocket.setCallback(this);
    }
}

void Isis::handleStartOperation(LifecycleOperation *operation)
{
    // TODO (adjacency phase): open an LLC socket on each participating
    // interface (SAP 0xFE), subscribe to the AllL1ISs/AllL2ISs multicast MACs,
    // and start sending IIH Hello PDUs.
}

void Isis::handleStopOperation(LifecycleOperation *operation)
{
    if (llcSocket.isOpen())
        llcSocket.close();
}

void Isis::handleCrashOperation(LifecycleOperation *operation)
{
    if (llcSocket.isOpen())
        llcSocket.destroy();
}

void Isis::handleMessageWhenUp(cMessage *msg)
{
    if (msg->isSelfMessage())
        throw cRuntimeError("Unexpected self message '%s'", msg->getName());
    else if (llcSocket.belongsToSocket(msg))
        llcSocket.processMessage(msg);
    else
        throw cRuntimeError("Unknown message '%s'", msg->getName());
}

void Isis::socketDataArrived(Ieee8022LlcSocket *socket, Packet *packet)
{
    // TODO (adjacency phase): dispatch the received IS-IS PDU by type.
    delete packet;
}

void Isis::socketClosed(Ieee8022LlcSocket *socket)
{
}

} // namespace isis
} // namespace inet
