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

#include "inet/applications/common/SocketTag_m.h"
#include "inet/applications/tcpapp/TcpSinkApp.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/common/packet/Message.h"
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

