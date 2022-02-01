//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/applications/tcpapp/TcpEchoApp.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/Simsignals.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/packet/Packet_m.h"
#include "inet/common/socket/SocketTag_m.h"
#include "inet/transportlayer/contract/tcp/TcpCommand_m.h"

namespace inet {

Define_Module(TcpEchoApp);

Define_Module(TcpEchoAppThread);

TcpEchoApp::TcpEchoApp()
{
}

TcpEchoApp::~TcpEchoApp()
{
}

void TcpEchoApp::initialize(int stage)
{
    TcpServerHostApp::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        delay = par("echoDelay");
        echoFactor = par("echoFactor");

        bytesRcvd = bytesSent = 0;
        WATCH(bytesRcvd);
        WATCH(bytesSent);
    }
}

void TcpEchoApp::sendDown(Packet *msg)
{
    if (msg->isPacket()) {
        Packet *pk = static_cast<Packet *>(msg);
        bytesSent += pk->getByteLength();
        emit(packetSentSignal, pk);
    }

    msg->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(&Protocol::tcp);
    msg->getTag<SocketReq>();
    send(msg, "socketOut");
}

void TcpEchoApp::refreshDisplay() const
{
    ApplicationBase::refreshDisplay();

    char buf[160];
    sprintf(buf, "threads: %d\nrcvd: %ld bytes\nsent: %ld bytes", socketMap.size(), bytesRcvd, bytesSent);
    getDisplayString().setTagArg("t", 0, buf);
}

void TcpEchoApp::finish()
{
    TcpServerHostApp::finish();

    recordScalar("bytesRcvd", bytesRcvd);
    recordScalar("bytesSent", bytesSent);
}

void TcpEchoAppThread::established()
{
}

void TcpEchoAppThread::dataArrived(Packet *rcvdPkt, bool urgent)
{
    echoAppModule->emit(packetReceivedSignal, rcvdPkt);
    int64_t rcvdBytes = rcvdPkt->getByteLength();
    echoAppModule->bytesRcvd += rcvdBytes;

    if (echoAppModule->echoFactor > 0 && sock->getState() == TcpSocket::CONNECTED) {
        Packet *outPkt = new Packet(rcvdPkt->getName(), TCP_C_SEND);
        // reverse direction, modify length, and send it back
        int socketId = rcvdPkt->getTag<SocketInd>()->getSocketId();
        outPkt->addTag<SocketReq>()->setSocketId(socketId);

        long outByteLen = rcvdBytes * echoAppModule->echoFactor;

        if (outByteLen < 1)
            outByteLen = 1;

        int64_t len = 0;
        for (; len + rcvdBytes <= outByteLen; len += rcvdBytes) {
            outPkt->insertAtBack(rcvdPkt->peekDataAt(B(0), B(rcvdBytes)));
        }
        if (len < outByteLen)
            outPkt->insertAtBack(rcvdPkt->peekDataAt(B(0), B(outByteLen - len)));

        ASSERT(outPkt->getByteLength() == outByteLen);

        if (echoAppModule->delay == 0)
            echoAppModule->sendDown(outPkt);
        else
            scheduleAfter(echoAppModule->delay, outPkt); // send after a delay
    }
    delete rcvdPkt;
}

/*
 * Called when a timer (scheduled via scheduleAt()) expires. To be redefined.
 */
void TcpEchoAppThread::timerExpired(cMessage *timer)
{
    Packet *pkt = check_and_cast<Packet *>(timer);
    pkt->setContextPointer(nullptr);
    echoAppModule->sendDown(pkt);
}

} // namespace inet

