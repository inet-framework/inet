//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
// Copyright (C) 2004-2005 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//


#include "IPvXTrafGen.h"

#include "AddressResolver.h"
#include "IPSocket.h"
#include "IPv4ControlInfo.h"
#include "IPv6ControlInfo.h"
#include "ModuleAccess.h"
#include "NodeOperations.h"

Define_Module(IPvXTrafSink);


simsignal_t IPvXTrafSink::rcvdPkSignal = registerSignal("rcvdPk");


void IPvXTrafSink::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_LOCAL)
    {
        numReceived = 0;
        WATCH(numReceived);
    }
    else if (stage == INITSTAGE_APPLICATION_LAYER)
    {
        int protocol = par("protocol");
        IPSocket ipSocket(gate("ipOut"));
        ipSocket.registerProtocol(protocol);

        NodeStatus *nodeStatus = dynamic_cast<NodeStatus *>(findContainingNode(this)->getSubmodule("status"));
        isOperational = (!nodeStatus) || nodeStatus->getState() == NodeStatus::UP;
    }
}

void IPvXTrafSink::handleMessage(cMessage *msg)
{
    if (!isOperational)
    {
        EV << "Module is down, received " << msg->getName() << " message dropped\n";
        delete msg;
        return;
    }
    processPacket(check_and_cast<cPacket *>(msg));

    if (ev.isGUI())
    {
        char buf[32];
        sprintf(buf, "rcvd: %d pks", numReceived);
        getDisplayString().setTagArg("t", 0, buf);
    }
}

bool IPvXTrafSink::handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback)
{
    Enter_Method_Silent();
    if (dynamic_cast<NodeStartOperation *>(operation)) isOperational = true;
    else if (dynamic_cast<NodeShutdownOperation *>(operation)) isOperational = false;
    else if (dynamic_cast<NodeCrashOperation *>(operation)) isOperational = false;
    else throw cRuntimeError("Unsupported lifecycle operation '%s'", operation->getClassName());
    return true;
}

void IPvXTrafSink::printPacket(cPacket *msg)
{
    Address src, dest;
    int protocol = -1;

    if (dynamic_cast<IPv4ControlInfo *>(msg->getControlInfo()) != NULL)
    {
        IPv4ControlInfo *ctrl = (IPv4ControlInfo *)msg->getControlInfo();
        src = ctrl->getSrcAddr();
        dest = ctrl->getDestAddr();
        protocol = ctrl->getProtocol();
    }
    else if (dynamic_cast<IPv6ControlInfo *>(msg->getControlInfo()) != NULL)
    {
        IPv6ControlInfo *ctrl = (IPv6ControlInfo *)msg->getControlInfo();
        src = ctrl->getSrcAddr();
        dest = ctrl->getDestAddr();
        protocol = ctrl->getProtocol();
    }

    EV << msg << endl;
    EV << "Payload length: " << msg->getByteLength() << " bytes" << endl;

    if (protocol != -1)
        EV << "src: " << src << "  dest: " << dest << "  protocol=" << protocol << "\n";
}

void IPvXTrafSink::processPacket(cPacket *msg)
{
    emit(rcvdPkSignal, msg);
    EV << "Received packet: ";
    printPacket(msg);
    delete msg;
    numReceived++;
}

