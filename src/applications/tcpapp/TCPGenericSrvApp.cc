//
// Copyright 2004 Andras Varga
//
// This library is free software, you can redistribute it and/or modify
// it under  the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation;
// either version 2 of the License, or any later version.
// The library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//


#include "TCPGenericSrvApp.h"
#include "TCPSocket.h"
#include "TCPCommand_m.h"
#include "GenericAppMsg_m.h"


Define_Module(TCPGenericSrvApp);

simsignal_t TCPGenericSrvApp::rcvdPkSignal = SIMSIGNAL_NULL;
simsignal_t TCPGenericSrvApp::sentPkSignal = SIMSIGNAL_NULL;

void TCPGenericSrvApp::initialize()
{
    const char *localAddress = par("localAddress");
    int localPort = par("localPort");
    delay = par("replyDelay");
    maxMsgDelay = 0;

    //statistics
    msgsRcvd = msgsSent = bytesRcvd = bytesSent = 0;
    rcvdPkSignal = registerSignal("rcvdPk");
    sentPkSignal = registerSignal("sentPk");

    WATCH(msgsRcvd);
    WATCH(msgsSent);
    WATCH(bytesRcvd);
    WATCH(bytesSent);

    TCPSocket socket;
    socket.setOutputGate(gate("tcpOut"));
    socket.setDataTransferMode(TCP_TRANSFER_OBJECT);
    socket.bind(localAddress[0] ? IPvXAddress(localAddress) : IPvXAddress(), localPort);
    socket.listen();
}

void TCPGenericSrvApp::sendOrSchedule(cMessage *msg, simtime_t delay)
{
    if (delay==0)
        sendBack(msg);
    else
        scheduleAt(simTime()+delay, msg);
}

void TCPGenericSrvApp::sendBack(cMessage *msg)
{
    GenericAppMsg *appmsg = dynamic_cast<GenericAppMsg*>(msg);

    if (appmsg)
    {
        msgsSent++;
        bytesSent += appmsg->getByteLength();
        emit(sentPkSignal, appmsg);

        EV << "sending \"" << appmsg->getName() << "\" to TCP, " << appmsg->getByteLength() << " bytes\n";
    }
    else
    {
        EV << "sending \"" << msg->getName() << "\" to TCP\n";
    }

    send(msg, "tcpOut");
}

void TCPGenericSrvApp::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage())
    {
        sendBack(msg);
    }
    else if (msg->getKind()==TCP_I_PEER_CLOSED)
    {
        // we'll close too, but only after there's surely no message
        // pending to be sent back in this connection
        msg->setName("close");
        msg->setKind(TCP_C_CLOSE);
        sendOrSchedule(msg, delay+maxMsgDelay);
    }
    else if (msg->getKind()==TCP_I_DATA || msg->getKind()==TCP_I_URGENT_DATA)
    {
        GenericAppMsg *appmsg = dynamic_cast<GenericAppMsg *>(msg);
        if (!appmsg)
            error("Message (%s)%s is not a GenericAppMsg -- "
                  "probably wrong client app, or wrong setting of TCP's "
                  "dataTransferMode parameters "
                  "(try \"object\")",
                  msg->getClassName(), msg->getName());

        msgsRcvd++;
        bytesRcvd += appmsg->getByteLength();
        emit(rcvdPkSignal, appmsg);

        long requestedBytes = appmsg->getExpectedReplyLength();

        simtime_t msgDelay = appmsg->getReplyDelay();
        if (msgDelay>maxMsgDelay)
            maxMsgDelay = msgDelay;

        bool doClose = appmsg->getServerClose();
        int connId = check_and_cast<TCPCommand *>(appmsg->getControlInfo())->getConnId();

        if (requestedBytes==0)
        {
            delete msg;
        }
        else
        {
            delete appmsg->removeControlInfo();
            TCPSendCommand *cmd = new TCPSendCommand();
            cmd->setConnId(connId);
            appmsg->setControlInfo(cmd);

            // set length and send it back
            appmsg->setKind(TCP_C_SEND);
            appmsg->setByteLength(requestedBytes);
            sendOrSchedule(appmsg, delay+msgDelay);
        }

        if (doClose)
        {
            cMessage *msg = new cMessage("close");
            msg->setKind(TCP_C_CLOSE);
            TCPCommand *cmd = new TCPCommand();
            cmd->setConnId(connId);
            msg->setControlInfo(cmd);
            sendOrSchedule(msg, delay+maxMsgDelay);
        }
    }
    else
    {
        // some indication -- ignore
        EV << "drop msg: " << msg->getName() << ", kind:"<< msg->getKind() << endl;
        delete msg;
    }

    if (ev.isGUI())
    {
        char buf[64];
        sprintf(buf, "rcvd: %ld pks %ld bytes\nsent: %ld pks %ld bytes", msgsRcvd, bytesRcvd, msgsSent, bytesSent);
        getDisplayString().setTagArg("t", 0, buf);
    }
}

void TCPGenericSrvApp::finish()
{
    EV << getFullPath() << ": sent " << bytesSent << " bytes in " << msgsSent << " packets\n";
    EV << getFullPath() << ": received " << bytesRcvd << " bytes in " << msgsRcvd << " packets\n";
}
