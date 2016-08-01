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

#include "inet/common/RawPacket.h"
#include "inet/transportlayer/contract/tcp/TCPCommand_m.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/lifecycle/NodeOperations.h"

namespace inet {

Define_Module(TCPEchoApp);

Register_Class(TCPEchoAppThread);

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
        bytesSent += ((cPacket *)msg)->getByteLength();
        emit(sentPkSignal, (cPacket *)msg);
    }

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
    cPacket *pkt = check_and_cast<cPacket *>(msg);
    echoAppModule->emit(echoAppModule->rcvdPkSignal, pkt);
    echoAppModule->bytesRcvd += pkt->getByteLength();

    if (echoAppModule->echoFactor == 0) {
        delete pkt;
    }
    else {
        // reverse direction, modify length, and send it back
        pkt->setKind(TCP_C_SEND);
        TCPCommand *ind = check_and_cast<TCPCommand *>(pkt->removeControlInfo());
        TCPSendCommand *cmd = new TCPSendCommand();
        cmd->setSocketId(ind->getSocketId());
        pkt->setControlInfo(cmd);
        delete ind;

        long byteLen = pkt->getByteLength() * echoAppModule->echoFactor;

        if (byteLen < 1)
            byteLen = 1;

        pkt->setByteLength(byteLen);

        RawPacket *baMsg = dynamic_cast<RawPacket *>(pkt);

        // if (dataTransferMode == TCP_TRANSFER_BYTESTREAM)
        if (baMsg) {
            ByteArray& outdata = baMsg->getByteArray();
            ByteArray indata = outdata;
            outdata.setDataArraySize(byteLen);

            for (long i = 0; i < byteLen; i++)
                outdata.setData(i, indata.getData(i / echoAppModule->echoFactor));
        }

        if (echoAppModule->delay == 0)
            echoAppModule->sendDown(pkt);
        else
            scheduleAt(simTime() + echoAppModule->delay, pkt); // send after a delay
    }
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

