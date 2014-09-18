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

#include "inet/applications/tcpapp/TCPEchoApp.h"

#include "inet/common/ByteArrayMessage.h"
#include "inet/transportlayer/contract/tcp/TCPCommand_m.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/lifecycle/NodeOperations.h"

namespace inet {

Define_Module(TCPEchoApp);

simsignal_t TCPEchoApp::rcvdPkSignal = registerSignal("rcvdPk");
simsignal_t TCPEchoApp::sentPkSignal = registerSignal("sentPk");

TCPEchoApp::TCPEchoApp()
{
    nodeStatus = NULL;
}

void TCPEchoApp::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        delay = par("echoDelay");
        echoFactor = par("echoFactor");

        bytesRcvd = bytesSent = 0;
        WATCH(bytesRcvd);
        WATCH(bytesSent);

        socket.setOutputGate(gate("tcpOut"));
        socket.readDataTransferModePar(*this);

        nodeStatus = dynamic_cast<NodeStatus *>(findContainingNode(this)->getSubmodule("status"));
    }
    else if (stage == INITSTAGE_APPLICATION_LAYER) {
        if (isNodeUp())
            startListening();
    }
}

bool TCPEchoApp::isNodeUp()
{
    return !nodeStatus || nodeStatus->getState() == NodeStatus::UP;
}

void TCPEchoApp::startListening()
{
    const char *localAddress = par("localAddress");
    int localPort = par("localPort");
    socket.renewSocket();
    socket.bind(localAddress[0] ? L3Address(localAddress) : L3Address(), localPort);
    socket.listen();
}

void TCPEchoApp::stopListening()
{
    socket.close();
}

void TCPEchoApp::sendDown(cMessage *msg)
{
    if (msg->isPacket()) {
        bytesSent += ((cPacket *)msg)->getByteLength();
        emit(sentPkSignal, (cPacket *)msg);
    }

    send(msg, "tcpOut");
}

void TCPEchoApp::handleMessage(cMessage *msg)
{
    if (!isNodeUp())
        throw cRuntimeError("Application is not running");
    if (msg->isSelfMessage()) {
        sendDown(msg);
    }
    else if (msg->getKind() == TCP_I_PEER_CLOSED) {
        // we'll close too
        msg->setName("close");
        msg->setKind(TCP_C_CLOSE);

        if (delay == 0)
            sendDown(msg);
        else
            scheduleAt(simTime() + delay, msg); // send after a delay
    }
    else if (msg->getKind() == TCP_I_DATA || msg->getKind() == TCP_I_URGENT_DATA) {
        cPacket *pkt = check_and_cast<cPacket *>(msg);
        emit(rcvdPkSignal, pkt);
        bytesRcvd += pkt->getByteLength();

        if (echoFactor == 0) {
            delete pkt;
        }
        else {
            // reverse direction, modify length, and send it back
            pkt->setKind(TCP_C_SEND);
            TCPCommand *ind = check_and_cast<TCPCommand *>(pkt->removeControlInfo());
            TCPSendCommand *cmd = new TCPSendCommand();
            cmd->setConnId(ind->getConnId());
            pkt->setControlInfo(cmd);
            delete ind;

            long byteLen = pkt->getByteLength() * echoFactor;

            if (byteLen < 1)
                byteLen = 1;

            pkt->setByteLength(byteLen);

            ByteArrayMessage *baMsg = dynamic_cast<ByteArrayMessage *>(pkt);

            // if (dataTransferMode == TCP_TRANSFER_BYTESTREAM)
            if (baMsg) {
                ByteArray& outdata = baMsg->getByteArray();
                ByteArray indata = outdata;
                outdata.setDataArraySize(byteLen);

                for (long i = 0; i < byteLen; i++)
                    outdata.setData(i, indata.getData(i / echoFactor));
            }

            if (delay == 0)
                sendDown(pkt);
            else
                scheduleAt(simTime() + delay, pkt); // send after a delay
        }
    }
    else {
        // some indication -- ignore
        delete msg;
    }

    if (ev.isGUI()) {
        char buf[80];
        sprintf(buf, "rcvd: %ld bytes\nsent: %ld bytes", bytesRcvd, bytesSent);
        getDisplayString().setTagArg("t", 0, buf);
    }
}

bool TCPEchoApp::handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback)
{
    Enter_Method_Silent();
    if (dynamic_cast<NodeStartOperation *>(operation)) {
        if (stage == NodeStartOperation::STAGE_APPLICATION_LAYER)
            startListening();
    }
    else if (dynamic_cast<NodeShutdownOperation *>(operation)) {
        if (stage == NodeShutdownOperation::STAGE_APPLICATION_LAYER)
            // TODO: wait until socket is closed
            stopListening();
    }
    else if (dynamic_cast<NodeCrashOperation *>(operation))
        ;
    else
        throw cRuntimeError("Unsupported lifecycle operation '%s'", operation->getClassName());
    return true;
}

void TCPEchoApp::finish()
{
    recordScalar("bytesRcvd", bytesRcvd);
    recordScalar("bytesSent", bytesSent);
}

} // namespace inet

