//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/applications/tcpapp/TcpEchoApp.h"

#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/packet/Packet_m.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/Simsignals.h"
#include "inet/common/socket/SocketTag_m.h"
#include "inet/common/TimeTag_m.h"
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
        socketSink.reference(gate("socketOut"), true);
        delay = par("echoDelay");
        echoFactor = par("echoFactor");

        bytesRcvd = bytesSent = 0;
        WATCH(bytesRcvd);
        WATCH(bytesSent);
    }
}

void TcpEchoApp::sendDown(Packet *msg)
{
    Enter_Method("sendDown");
    take(msg);
    bytesSent += msg->getByteLength();
    msg->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(&Protocol::tcp);
    msg->getTag<SocketReq>();
    socketSink.pushPacket(msg);
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
    cancelAndDelete(readDelayTimer);
    cancelAndDelete(delayedPacket);
}

void TcpEchoAppThread::sendOrScheduleReadCommandIfNeeded()
{
    if (!sock->getAutoRead() && sock->isOpen()) {
        simtime_t delay = hostmod->par("readDelay");
        if (delay >= SIMTIME_ZERO) {
            if (readDelayTimer == nullptr)
                readDelayTimer = new cMessage("readDelayTimer");
            scheduleAfter(delay, readDelayTimer);
        }
        else {
            // send read message to TCP
            read();
        }
    }
}

void TcpEchoAppThread::established()
{
    Enter_Method("established");
    sendOrScheduleReadCommandIfNeeded();
}

void TcpEchoAppThread::dataArrived(Packet *rcvdPkt, bool urgent)
{
    Enter_Method("dataArrived");
    take(rcvdPkt);
    emit(packetReceivedSignal, rcvdPkt);
    int64_t rcvdBytes = rcvdPkt->getByteLength();
    echoAppModule->bytesRcvd += rcvdBytes;

    if (sock->getState() != TcpSocket::CONNECTED) {
    }
    else if (echoAppModule->echoFactor > 0.0) {
        Packet *outPkt = new Packet(rcvdPkt->getName(), TCP_C_SEND);
        // reverse direction, modify length, and send it back
        int socketId = rcvdPkt->getTag<SocketInd>()->getSocketId();
        outPkt->addTag<SocketReq>()->setSocketId(socketId);

        if (echoAppModule->echoFactor == 1.0) {
            auto content = rcvdPkt->peekDataAt(B(0), B(rcvdBytes))->dupShared();
            content->removeTagsWherePresent<CreationTimeTag>(b(0), content->getChunkLength());
            content->addTag<CreationTimeTag>()->setCreationTime(simTime());
            outPkt->insertAtBack(content);
        }
        else {
            int64_t outByteLen = rcvdBytes * echoAppModule->echoFactor;
            if (outByteLen < 1)
                outByteLen = 1;
            auto content = makeShared<ByteCountChunk>(B(outByteLen));
            content->addTag<CreationTimeTag>()->setCreationTime(simTime());
            outPkt->insertAtBack(content);
        }
        if (echoAppModule->delay == 0) {
            sendDown(outPkt);
            sendOrScheduleReadCommandIfNeeded();
        }
        else {
            delayedPacket = outPkt;
            scheduleAfter(echoAppModule->delay, outPkt); // send after a delay
        }
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
    ASSERT(getSimulation()->getContext() == this);

    if (timer == readDelayTimer) {
        // send read message to TCP
        read();
    }
    else if (timer == delayedPacket) {
        sendDown(delayedPacket);
        delayedPacket = nullptr;
        sendOrScheduleReadCommandIfNeeded();
    }
    else
        throw cRuntimeError("Model error: unknown timer message arrived");
}

void TcpEchoAppThread::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage())
        timerExpired(msg);
    else
        throw cRuntimeError("Model error: allows only self messages");
}

void TcpEchoAppThread::init(TcpServerHostApp *hostmodule, TcpSocket *socket)
{
    TcpServerThreadBase::init(hostmodule, socket);
    echoAppModule = check_and_cast<TcpEchoApp *>(hostmod);
}

void TcpEchoAppThread::close()
{
    Enter_Method("close");
    cancelAndDelete(readDelayTimer);
    readDelayTimer = nullptr;
    cancelAndDelete(delayedPacket);
    delayedPacket = nullptr;
    TcpServerThreadBase::close();
}

void TcpEchoAppThread::sendDown(Packet *msg)
{
    emit(packetSentSignal, msg);
    drop(msg);
    echoAppModule->sendDown(msg);
}

void TcpEchoAppThread::read()
{
    omnetpp::cMethodCallContextSwitcher __ctx(echoAppModule);
    __ctx.methodCall("TcpSocket::read");
    sock->read(hostmod->par("readSize"));
}

} // namespace inet

