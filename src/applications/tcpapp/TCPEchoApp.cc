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


#include "TCPEchoApp.h"
#include "TCPSocket.h"
#include "TCPCommand_m.h"


Define_Module(TCPEchoApp);

TCPEchoApp::TCPEchoApp()
{
    bytesRcvdVector = bytesSentVector = bytesSentAndAckedVector = NULL;
}

TCPEchoApp::~TCPEchoApp()
{
    delete bytesRcvdVector;
    delete bytesSentVector;
    delete bytesSentAndAckedVector;
}

void TCPEchoApp::initialize()
{
    cSimpleModule::initialize();
    const char *address = par("address");
    int port = par("port");
    delay = par("echoDelay");
    echoFactor = par("echoFactor");
    useExplicitRead = par("useExplicitRead");
    sendNotificationsEnabled = par("sendNotificationsEnabled");
    readBufferSize = par("receiveBufferSize");
    sendBufferLimit = par("sendBufferLimit");
    waitingData = false;
    bytesInSendQueue = 0;

    bytesRcvd = bytesSent = bytesSentAndAcked = 0;

    bytesRcvdVector = new cOutVector("bytesRcvd");
    bytesSentVector = new cOutVector("bytesSent");
    bytesSentAndAckedVector = new cOutVector("bytesSentAndAcked");

    WATCH(bytesRcvd);
    WATCH(bytesSent);
    WATCH(bytesSentAndAcked);
    WATCH(bytesInSendQueue);

    TCPSocket socket;
    socket.setOutputGate(gate("tcpOut"));
    socket.readDataTransferModePar(*this);
    socket.setExplicitReads(useExplicitRead);
    socket.setSendNotifications(sendNotificationsEnabled);
    socket.setReceiveBufferSize(readBufferSize);
    socket.bind(address[0] ? IPvXAddress(address) : IPvXAddress(), port);
    socket.listen();
}

void TCPEchoApp::sendDown(cMessage *msg)
{
    ASSERT(msg);
    if (msg->getKind() ==TCP_C_SEND && msg->isPacket())
    {
        int64 len = ((cPacket *)msg)->getByteLength();
        bytesInSendQueue += len;
        bytesSent += len;
        bytesSentVector->record(bytesSent);
    }
    send(msg, "tcpOut");
}

void TCPEchoApp::sendDownReadCmd(cMessage *msg, int connId)
{
    long bytes = (sendBufferLimit - bytesInSendQueue) / 4;

    if (bytes > 0)
    {
        if (!msg)
            msg = new cMessage();

        msg->setKind(TCP_C_READ);
        msg->setName("READ");
        TCPReadCommand *cmd = new TCPReadCommand();
        cmd->setConnId(connId);
        cmd->setBytes(bytes);
        msg->setControlInfo(cmd);
        waitingData = true;
        send(msg, "tcpOut");
    }
}

void TCPEchoApp::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage())
    {
        sendDown(msg);
    }
    else
    {
        switch (msg->getKind())
        {
            case TCP_I_PEER_CLOSED:
                msg->setKind(TCP_C_CLOSE);
                if (delay==0)
                    sendDown(msg);
                else
                    scheduleAt(simTime()+delay, msg); // send after a delay
                break;

            case TCP_I_DATA:
            case TCP_I_URGENT_DATA:
            {
                waitingData = false;
                cPacket *pkt = check_and_cast<cPacket *>(msg);
                bytesRcvd += pkt->getByteLength();
                bytesRcvdVector->record(bytesRcvd);
                TCPCommand *ind = check_and_cast<TCPCommand *>(pkt->removeControlInfo());
                int connId = ind->getConnId();
                delete ind;

                if (echoFactor==0)
                {
                    delete pkt;
                }
                else
                {
                    // reverse direction, modify length, and send it back
                    pkt->setKind(TCP_C_SEND);
                    TCPSendCommand *cmd = new TCPSendCommand();
                    cmd->setConnId(connId);
                    pkt->setControlInfo(cmd);

                    long byteLen = pkt->getByteLength()*echoFactor;
                    if (byteLen<1) byteLen=1;
                    pkt->setByteLength(byteLen);

                    if (delay==0)
                        sendDown(pkt);
                    else
                        scheduleAt(simTime()+delay, pkt); // send after a delay
                }
                if (checkForRead())
                    sendDownReadCmd(NULL, connId);
                break;
            }
            ASSERT(0);

            case TCP_I_DATA_ARRIVED:
            {
                ASSERT(useExplicitRead);
                TCPDataArrivedInfo *ind = check_and_cast<TCPDataArrivedInfo *>(msg->removeControlInfo());
                int connId = ind->getConnId();
                delete ind;
                if (checkForRead())
                    sendDownReadCmd(msg, connId);
                else
                    delete msg;
                break;
            }
            ASSERT(0);

            case TCP_I_DATA_SENT:
            {
                ASSERT(sendNotificationsEnabled);
                TCPDataSentInfo *ind = check_and_cast<TCPDataSentInfo *>(msg->removeControlInfo());
                bytesSentAndAcked += ind->getSentBytes();
                bytesSentAndAckedVector->record(bytesSentAndAcked);
                bytesInSendQueue = ind->getAvailableBytesInSendQueue();
                int connId = ind->getConnId();
                delete ind;
                if (checkForRead())
                    sendDownReadCmd(msg, connId);
                else
                    delete msg;
                break;
            }
            ASSERT(0);

            default:
                delete msg;
                break;
        }
    }

    if (ev.isGUI())
    {
        char buf[80];
        sprintf(buf, "rcvd: %ld bytes\nsent: %ld bytes \nsent&ack: %ld", bytesRcvd, bytesSent, bytesSentAndAcked);
        getDisplayString().setTagArg("t",0,buf);
    }
}

void TCPEchoApp::finish()
{
    recordScalar("bytesRcvd", bytesRcvd);
    recordScalar("bytesSent", bytesSent);
}
