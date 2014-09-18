//
// Copyright (C) 2005 Andras Varga
//
// Based on the video streaming app of the similar name by Johnny Lai.
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

#include "inet/applications/udpapp/UDPVideoStreamCli.h"

#include "inet/transportlayer/contract/udp/UDPControlInfo_m.h"
#include "inet/networklayer/common/L3AddressResolver.h"

namespace inet {

Define_Module(UDPVideoStreamCli);

simsignal_t UDPVideoStreamCli::rcvdPkSignal = registerSignal("rcvdPk");

void UDPVideoStreamCli::initialize(int stage)
{
    ApplicationBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        selfMsg = new cMessage("UDPVideoStreamStart");
    }
}

void UDPVideoStreamCli::finish()
{
    ApplicationBase::finish();
}

void UDPVideoStreamCli::handleMessageWhenUp(cMessage *msg)
{
    if (msg->isSelfMessage()) {
        requestStream();
    }
    else if (msg->getKind() == UDP_I_DATA) {
        // process incoming packet
        receiveStream(PK(msg));
    }
    else if (msg->getKind() == UDP_I_ERROR) {
        EV_WARN << "Ignoring UDP error report\n";
        delete msg;
    }
    else {
        throw cRuntimeError("Unrecognized message (%s)%s", msg->getClassName(), msg->getName());
    }
}

void UDPVideoStreamCli::requestStream()
{
    int svrPort = par("serverPort");
    int localPort = par("localPort");
    const char *address = par("serverAddress");
    L3Address svrAddr = L3AddressResolver().resolve(address);

    if (svrAddr.isUnspecified()) {
        EV_ERROR << "Server address is unspecified, skip sending video stream request\n";
        return;
    }

    EV_INFO << "Requesting video stream from " << svrAddr << ":" << svrPort << "\n";

    socket.setOutputGate(gate("udpOut"));
    socket.bind(localPort);

    cPacket *pk = new cPacket("VideoStrmReq");
    socket.sendTo(pk, svrAddr, svrPort);
}

void UDPVideoStreamCli::receiveStream(cPacket *pk)
{
    EV_INFO << "Video stream packet: " << UDPSocket::getReceivedPacketInfo(pk) << endl;
    emit(rcvdPkSignal, pk);
    delete pk;
}

bool UDPVideoStreamCli::handleNodeStart(IDoneCallback *doneCallback)
{
    simtime_t startTimePar = par("startTime");
    simtime_t startTime = std::max(startTimePar, simTime());
    scheduleAt(startTime, selfMsg);
    return true;
}

bool UDPVideoStreamCli::handleNodeShutdown(IDoneCallback *doneCallback)
{
    cancelEvent(selfMsg);
    //TODO if(socket.isOpened()) socket.close();
    return true;
}

void UDPVideoStreamCli::handleNodeCrash()
{
    cancelEvent(selfMsg);
}

} // namespace inet

