//
// Copyright (C) 2004 Andras Varga
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

#include "inet/applications/tcpapp/TCPSinkApp.h"

#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/transportlayer/contract/tcp/TCPSocket.h"

namespace inet {

Define_Module(TCPSinkApp);

simsignal_t TCPSinkApp::rcvdPkSignal = registerSignal("rcvdPk");

void TCPSinkApp::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        bytesRcvd = 0;
        WATCH(bytesRcvd);
    }
    else if (stage == INITSTAGE_APPLICATION_LAYER) {
        bool isOperational;
        NodeStatus *nodeStatus = dynamic_cast<NodeStatus *>(findContainingNode(this)->getSubmodule("status"));
        isOperational = (!nodeStatus) || nodeStatus->getState() == NodeStatus::UP;
        if (!isOperational)
            throw cRuntimeError("This module doesn't support starting in node DOWN state");

        const char *localAddress = par("localAddress");
        int localPort = par("localPort");
        TCPSocket socket;
        socket.setOutputGate(gate("tcpOut"));
        socket.readDataTransferModePar(*this);
        socket.bind(localAddress[0] ? L3AddressResolver().resolve(localAddress) : L3Address(), localPort);
        socket.listen();
    }
}

void TCPSinkApp::handleMessage(cMessage *msg)
{
    if (msg->getKind() == TCP_I_PEER_CLOSED) {
        // we close too
        msg->setName("close");
        msg->setKind(TCP_C_CLOSE);
        send(msg, "tcpOut");
    }
    else if (msg->getKind() == TCP_I_DATA || msg->getKind() == TCP_I_URGENT_DATA) {
        cPacket *pk = PK(msg);
        long packetLength = pk->getByteLength();
        bytesRcvd += packetLength;
        emit(rcvdPkSignal, pk);
        delete msg;

        if (ev.isGUI()) {
            char buf[32];
            sprintf(buf, "rcvd: %ld bytes", bytesRcvd);
            getDisplayString().setTagArg("t", 0, buf);
        }
    }
    else {
        // must be data or some kind of indication -- can be dropped
        delete msg;
    }
}

void TCPSinkApp::finish()
{
}

} // namespace inet

