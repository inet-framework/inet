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
    bytesSent += msg->getByteLength();
    emit(packetSentSignal, msg);
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

TcpEchoAppThread::~TcpEchoAppThread()
{
    hostmod->cancelAndDelete(readDelayTimer);
}

void TcpEchoAppThread::sendOrScheduleReadCommandIfNeeded()
{
    if (!sock->getAutoRead() && sock->isOpen()) {
        simtime_t delay = hostmod->par("readDelay");
        if (delay >= SIMTIME_ZERO) {
            if (readDelayTimer == nullptr) {
                readDelayTimer = new cMessage("readDelayTimer");
                readDelayTimer->setContextPointer(this);
            }
            hostmod->scheduleAfter(delay, readDelayTimer);
        }
        else {
            // send read message to TCP
            sock->read(hostmod->par("readSize"));
        }
    }
}

void TcpEchoAppThread::established()
{
    sendOrScheduleReadCommandIfNeeded();
}

void TcpEchoAppThread::dataArrived(Packet *rcvdPkt, bool urgent)
{
    echoAppModule->emit(packetReceivedSignal, rcvdPkt);
    int64_t rcvdBytes = rcvdPkt->getByteLength();
    echoAppModule->bytesRcvd += rcvdBytes;

    if (sock->getState() != TcpSocket::CONNECTED) {
    }
    else if (echoAppModule->echoFactor > 0) {
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

        if (echoAppModule->delay == 0) {
            echoAppModule->sendDown(outPkt);
            sendOrScheduleReadCommandIfNeeded();
        }
        else
            scheduleAfter(echoAppModule->delay, outPkt); // send after a delay
    }
    else {
        sendOrScheduleReadCommandIfNeeded();
    }
    delete rcvdPkt;
}

/*
 * Called when a timer (scheduled via scheduleAt()) expires. To be redefined.
 */
void TcpEchoAppThread::timerExpired(cMessage *timer)
{
    if (timer == readDelayTimer) {
        // send read message to TCP
        sock->read(this->hostmod->par("readSize"));
    }
    else if (Packet *pkt = check_and_cast<Packet *>(timer)) {
        pkt->setContextPointer(nullptr);
        echoAppModule->sendDown(pkt);
        sendOrScheduleReadCommandIfNeeded();
    }
    else
        throw cRuntimeError("Model error: unknown timer message arrived");
}

void TcpEchoAppThread::init(TcpServerHostApp *hostmodule, TcpSocket *socket)
{
    TcpServerThreadBase::init(hostmodule, socket);
    echoAppModule = check_and_cast<TcpEchoApp *>(hostmod);
}

void TcpEchoAppThread::close()
{
    hostmod->cancelAndDelete(readDelayTimer);
    TcpServerThreadBase::close();
}

} // namespace inet

