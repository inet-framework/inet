//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/applications/tcpapp/TcpSinkApp.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/common/packet/Message.h"
#include "inet/common/socket/SocketTag_m.h"
#include "inet/networklayer/common/L3AddressResolver.h"

namespace inet {

Define_Module(TcpSinkApp);
Define_Module(TcpSinkAppThread);

TcpSinkApp::TcpSinkApp()
{
}

TcpSinkApp::~TcpSinkApp()
{
}

void TcpSinkApp::initialize(int stage)
{
    TcpServerHostApp::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        bytesRcvd = 0;
        WATCH(bytesRcvd);
    }
}

void TcpSinkApp::refreshDisplay() const
{
    ApplicationBase::refreshDisplay();

    char buf[160];
    sprintf(buf, "threads: %d\nrcvd: %ld bytes", socketMap.size(), bytesRcvd);
    getDisplayString().setTagArg("t", 0, buf);
}

void TcpSinkApp::finish()
{
    TcpServerHostApp::finish();

    recordScalar("bytesRcvd", bytesRcvd);
}

void TcpSinkAppThread::initialize(int stage)
{
    TcpServerThreadBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        bytesRcvd = 0;
        WATCH(bytesRcvd);
    }
}

void TcpSinkAppThread::handleMessage(cMessage *msg)
{
    if(msg->isSelfMessage()) {
        timerExpired(msg);
    }
    else
        throw cRuntimeError("Received a non-self message.");
}

void TcpSinkAppThread::timerExpired(cMessage *timer)
{
    ASSERT(getSimulation()->getContext() == this);

    if (timer == readDelayTimer) {
        // send read message to TCP
        read();
    }
    else
        throw cRuntimeError("Model error: unknown timer message arrived");
}

void TcpSinkAppThread::sendOrScheduleReadCommandIfNeeded()
{
    if (!sock->getAutoRead() && sock->isOpen()) {
        simtime_t delay = hostmod->par("readDelay");
        if (delay >= SIMTIME_ZERO) {
            if (readDelayTimer == nullptr) {
                readDelayTimer = new cMessage("readDelayTimer");
                readDelayTimer->setContextPointer(this);
            }
            scheduleAfter(delay, readDelayTimer);
        }
        else {
            // send read message to TCP
            read();
        }
    }
}

void TcpSinkAppThread::established()
{
    Enter_Method("established");
    bytesRcvd = 0;
    sendOrScheduleReadCommandIfNeeded();
}

void TcpSinkAppThread::dataArrived(Packet *pk, bool urgent)
{
    Enter_Method("dataArrived");
    take(pk);
    long packetLength = pk->getByteLength();
    bytesRcvd += packetLength;
    sinkAppModule->bytesRcvd += packetLength;

    emit(packetReceivedSignal, pk);
    delete pk;
    sendOrScheduleReadCommandIfNeeded();
}

void TcpSinkAppThread::refreshDisplay() const
{
    TcpServerThreadBase::refreshDisplay();
    
    std::ostringstream os;
    os << (sock ? TcpSocket::stateName(sock->getState()) : "NULL_SOCKET") << "\nrcvd: " << bytesRcvd << " bytes";
    getDisplayString().setTagArg("t", 0, os.str().c_str());
}

void TcpSinkAppThread::read()
{
    omnetpp::cMethodCallContextSwitcher __ctx(hostmod);
    __ctx.methodCall("TcpSocket::read");
    sock->read(hostmod->par("readSize"));
}

} // namespace inet

