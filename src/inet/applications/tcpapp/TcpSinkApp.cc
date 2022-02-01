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

void TcpSinkAppThread::established()
{
    bytesRcvd = 0;
}

void TcpSinkAppThread::dataArrived(Packet *pk, bool urgent)
{
    long packetLength = pk->getByteLength();
    bytesRcvd += packetLength;
    sinkAppModule->bytesRcvd += packetLength;

    emit(packetReceivedSignal, pk);
    delete pk;
}

void TcpSinkAppThread::refreshDisplay() const
{
    std::ostringstream os;
    os << (sock ? TcpSocket::stateName(sock->getState()) : "NULL_SOCKET") << "\nrcvd: " << bytesRcvd << " bytes";
    getDisplayString().setTagArg("t", 0, os.str().c_str());
}

} // namespace inet

