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
#include "inet/applications/tcpapp/TCPEchoApp.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/lifecycle/NodeOperations.h"
#include "inet/common/packet/Packet_m.h"
#include "inet/transportlayer/contract/tcp/TCPCommand_m.h"

namespace inet {

Define_Module(TCPEchoApp);

Define_Module(TCPEchoAppThread);

simsignal_t TCPEchoApp::rcvdPkSignal = registerSignal("rcvdPk");
simsignal_t TCPEchoApp::sentPkSignal = registerSignal("sentPk");

TCPEchoApp::TCPEchoApp()
{
}

TCPEchoApp::~TCPEchoApp()
{
}

void TCPEchoApp::initialize(int stage)
{
    TCPSrvHostApp::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        delay = par("echoDelay");
        echoFactor = par("echoFactor");

        bytesRcvd = bytesSent = 0;
        WATCH(bytesRcvd);
        WATCH(bytesSent);
    }
}

void TCPEchoApp::sendDown(cMessage *msg)
{
    if (msg->isPacket()) {
        cPacket *pk = static_cast<cPacket *>(msg);
        bytesSent += pk->getByteLength();
        emit(sentPkSignal, pk);
    }

    msg->ensureTag<DispatchProtocolReq>()->setProtocol(&Protocol::tcp);
    msg->getMandatoryTag<SocketReq>();
    send(msg, "socketOut");
}

void TCPEchoApp::refreshDisplay() const
{
    char buf[160];
    sprintf(buf, "threads: %d\nrcvd: %ld bytes\nsent: %ld bytes", socketMap.size(), bytesRcvd, bytesSent);
    getDisplayString().setTagArg("t", 0, buf);
}


void TCPEchoApp::finish()
{
    recordScalar("bytesRcvd", bytesRcvd);
    recordScalar("bytesSent", bytesSent);
}

void TCPEchoAppThread::established()
{
}

void TCPEchoAppThread::dataArrived(cMessage *msg, bool urgent)
{
    Packet *rcvdPkt = check_and_cast<Packet *>(msg);
    echoAppModule->emit(echoAppModule->rcvdPkSignal, rcvdPkt);
    int64_t rcvdBytes = rcvdPkt->getByteLength();
    echoAppModule->bytesRcvd += rcvdBytes;

    if (echoAppModule->echoFactor > 0) {
        Packet *outPkt = new Packet(rcvdPkt->getName());
        // reverse direction, modify length, and send it back
        outPkt->setKind(TCP_C_SEND);
        int socketId = rcvdPkt->getMandatoryTag<SocketInd>()->getSocketId();
        outPkt->ensureTag<SocketReq>()->setSocketId(socketId);

        long outByteLen = rcvdPkt->getByteLength() * echoAppModule->echoFactor;

        if (outByteLen < 1)
            outByteLen = 1;

        int64_t len = 0;
        for ( ; len + rcvdBytes <= outByteLen; len += rcvdBytes) {
            outPkt->append(rcvdPkt->peekDataAt(0, rcvdBytes));
        }
        if (len < outByteLen)
            outPkt->append(rcvdPkt->peekDataAt(0, outByteLen - len));

        ASSERT(outPkt->getByteLength() == outByteLen);

        if (echoAppModule->delay == 0)
            echoAppModule->sendDown(outPkt);
        else
            scheduleAt(simTime() + echoAppModule->delay, outPkt); // send after a delay
    }
    delete rcvdPkt;
}

  /*
   * Called when a timer (scheduled via scheduleAt()) expires. To be redefined.
   */
void TCPEchoAppThread::timerExpired(cMessage *timer)
{
    cPacket *pkt = PK(timer);
    pkt->setContextPointer(nullptr);
    echoAppModule->sendDown(pkt);
}

} // namespace inet

