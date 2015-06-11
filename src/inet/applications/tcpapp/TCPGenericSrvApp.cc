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

#include "inet/applications/tcpapp/TCPGenericSrvApp.h"

#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/transportlayer/contract/tcp/TCPSocket.h"
#include "inet/transportlayer/contract/tcp/TCPCommand_m.h"
#include "GenericAppMsg_m.h"

namespace inet {

Define_Module(TCPGenericSrvApp);

simsignal_t TCPGenericSrvApp::rcvdPkSignal = registerSignal("rcvdPk");
simsignal_t TCPGenericSrvApp::sentPkSignal = registerSignal("sentPk");

void TCPGenericSrvApp::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        delay = par("replyDelay");
        maxMsgDelay = 0;

        //statistics
        msgsRcvd = msgsSent = bytesRcvd = bytesSent = 0;

        WATCH(msgsRcvd);
        WATCH(msgsSent);
        WATCH(bytesRcvd);
        WATCH(bytesSent);
    }
    else if (stage == INITSTAGE_APPLICATION_LAYER) {
        const char *localAddress = par("localAddress");
        int localPort = par("localPort");
        TCPSocket socket;
        socket.setOutputGate(gate("tcpOut"));
        socket.setDataTransferMode(TCP_TRANSFER_OBJECT);
        socket.bind(localAddress[0] ? L3AddressResolver().resolve(localAddress) : L3Address(), localPort);
        socket.listen();

        bool isOperational;
        NodeStatus *nodeStatus = dynamic_cast<NodeStatus *>(findContainingNode(this)->getSubmodule("status"));
        isOperational = (!nodeStatus) || nodeStatus->getState() == NodeStatus::UP;
        if (!isOperational)
            throw cRuntimeError("This module doesn't support starting in node DOWN state");
    }
}

void TCPGenericSrvApp::sendOrSchedule(cMessage *msg, simtime_t delay)
{
    if (delay == 0)
        sendBack(msg);
    else
        scheduleAt(simTime() + delay, msg);
}

void TCPGenericSrvApp::sendBack(cMessage *msg)
{
    cPacket *packet = dynamic_cast<cPacket *>(msg);

    if (packet) {
        msgsSent++;
        bytesSent += packet->getByteLength();
        emit(sentPkSignal, packet);

        EV_INFO << "sending \"" << packet->getName() << "\" to TCP, " << packet->getByteLength() << " bytes\n";
    }
    else {
        EV_INFO << "sending \"" << msg->getName() << "\" to TCP\n";
    }

    send(msg, "tcpOut");
}

void TCPGenericSrvApp::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage()) {
        sendBack(msg);
    }
    else if (msg->getKind() == TCP_I_PEER_CLOSED) {
        // we'll close too, but only after there's surely no message
        // pending to be sent back in this connection
        msg->setName("close");
        msg->setKind(TCP_C_CLOSE);
        sendOrSchedule(msg, delay + maxMsgDelay);
    }
    else if (msg->getKind() == TCP_I_DATA || msg->getKind() == TCP_I_URGENT_DATA) {
        GenericAppMsg *appmsg = dynamic_cast<GenericAppMsg *>(msg);
        if (!appmsg)
            throw cRuntimeError("Message (%s)%s is not a GenericAppMsg -- "
                                "probably wrong client app, or wrong setting of TCP's "
                                "dataTransferMode parameters "
                                "(try \"object\")",
                    msg->getClassName(), msg->getName());

        msgsRcvd++;
        bytesRcvd += appmsg->getByteLength();
        emit(rcvdPkSignal, appmsg);

        long requestedBytes = appmsg->getExpectedReplyLength();

        simtime_t msgDelay = appmsg->getReplyDelay();
        if (msgDelay > maxMsgDelay)
            maxMsgDelay = msgDelay;

        bool doClose = appmsg->getServerClose();
        int connId = check_and_cast<TCPCommand *>(appmsg->getControlInfo())->getSocketId();

        if (requestedBytes == 0) {
            delete msg;
        }
        else {
            delete appmsg->removeControlInfo();
            TCPSendCommand *cmd = new TCPSendCommand();
            cmd->setSocketId(connId);
            appmsg->setControlInfo(cmd);

            // set length and send it back
            appmsg->setKind(TCP_C_SEND);
            appmsg->setByteLength(requestedBytes);
            sendOrSchedule(appmsg, delay + msgDelay);
        }

        if (doClose) {
            cMessage *msg = new cMessage("close");
            msg->setKind(TCP_C_CLOSE);
            TCPCommand *cmd = new TCPCommand();
            cmd->setSocketId(connId);
            msg->setControlInfo(cmd);
            sendOrSchedule(msg, delay + maxMsgDelay);
        }
    }
    else {
        // some indication -- ignore
        EV_WARN << "drop msg: " << msg->getName() << ", kind:" << msg->getKind() << "(" << cEnum::get("inet::TcpStatusInd")->getStringFor(msg->getKind()) << ")\n";
        delete msg;
    }

    if (hasGUI()) {
        char buf[64];
        sprintf(buf, "rcvd: %ld pks %ld bytes\nsent: %ld pks %ld bytes", msgsRcvd, bytesRcvd, msgsSent, bytesSent);
        getDisplayString().setTagArg("t", 0, buf);
    }
}

void TCPGenericSrvApp::finish()
{
    EV_INFO << getFullPath() << ": sent " << bytesSent << " bytes in " << msgsSent << " packets\n";
    EV_INFO << getFullPath() << ": received " << bytesRcvd << " bytes in " << msgsRcvd << " packets\n";
}

} // namespace inet

