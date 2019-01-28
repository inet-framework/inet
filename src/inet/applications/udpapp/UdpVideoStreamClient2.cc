//
// Copyright (C) 2005 Andras Varga
// Copyright (C) 2015 A. Ariza (Malaga University)
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


//
// based on the video streaming app of the similar name by Johnny Lai
//

#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/applications/udpapp/VideoPacket_m.h"
#include "inet/applications/udpapp/UdpVideoStreamClient2.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/packet/chunk/ByteCountChunk.h"
#include "inet/transportlayer/contract/udp/UdpControlInfo_m.h"

namespace inet {

Define_Module(UdpVideoStreamClient2);

simsignal_t UdpVideoStreamClient2::rcvdPkSignal = registerSignal("rcvdPk");

UdpVideoStreamClient2::UdpVideoStreamClient2()
{
    reintentTimer = new cMessage();
    timeOutMsg =  new cMessage();
    selfMsg = new cMessage("UDPVideoStreamStart");
    socketOpened = false;
    numPframes = 0;
    numIframes = 0;
    numBframes = 0;
    totalBytesI = 0;
    totalBytesP = 0;
    totalBytesB = 0;
    recieved = false;
}

UdpVideoStreamClient2::~UdpVideoStreamClient2()
{
    cancelAndDelete(reintentTimer);
    cancelAndDelete(timeOutMsg);
    cancelAndDelete(selfMsg);
}

void UdpVideoStreamClient2::initialize(int stage)
{
    ApplicationBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL)
    {
        // statistics
        timeOut  = par("timeOut");
        limitDelay = par("limitDelay");
    }
}

void UdpVideoStreamClient2::finish()
{
    recordScalar("Total received", numRecPackets);
    if (numPframes != 0 || numIframes != 0 ||  numBframes != 0)
    {
        recordScalar("Total I frames received", numIframes);
        recordScalar("Total P frames received", numPframes);
        recordScalar("Total B frames received", numBframes);
        recordScalar("Total I bytes received", totalBytesI);
        recordScalar("Total P bytes received", totalBytesP);
        recordScalar("Total B bytes received", totalBytesB);
    }
}

void UdpVideoStreamClient2::handleMessageWhenUp(cMessage* msg)
{
    if (msg->isSelfMessage())
    {
        if (reintentTimer == msg)
            requestStream();
        else if (timeOutMsg == msg)
            timeOutData();
        else
            requestStream();
    }
    else
        socket.processMessage(msg);
}

void UdpVideoStreamClient2::socketDataArrived(UdpSocket *socket, Packet *packet)
{
    // process incoming packet
    receiveStream(packet);
}

void UdpVideoStreamClient2::socketErrorArrived(UdpSocket *socket, Indication *indication)
{
    EV_WARN << "Ignoring UDP error report " << indication->getName() << endl;
    delete indication;
}

void UdpVideoStreamClient2::socketClosed(UdpSocket *socket)
{
    if (operationalState == State::STOPPING_OPERATION)
        startActiveOperationExtraTimeOrFinish(par("stopOperationExtraTime"));
}

void UdpVideoStreamClient2::requestStream()
{

    if (recieved && !par("multipleRequest").boolValue())
        return;
    int svrPort = par("serverPort");
    int localPort = par("localPort");
    const char *address = par("serverAddress");
    L3Address svrAddr = L3AddressResolver().resolve(address);

    if (svrAddr.isUnspecified())
    {
        EV << "Server address is unspecified, skip sending video stream request\n";
        return;
    }

    EV << "Requesting video stream from " << svrAddr << ":" << svrPort << "\n";

    if (!socketOpened)
    {
        socketOpened = true;
        socket.setOutputGate(gate("socketOut"));
        socket.bind(localPort);
        socket.setCallback(this);
        socket.setBroadcast(true);
    }

    lastSeqNum = -1;
    Packet *pk = new Packet("VideoStrmReq");
    const auto& payload = makeShared<ByteCountChunk>(B(1));    //FIXME set packet length
    pk->insertAtBack(payload);
    socket.sendTo(pk, svrAddr, svrPort);


    double reint = par("reintent").intValue();
    if (reint > 0)
        scheduleAt(simTime()+par("reintent").intValue(), reintentTimer);
}

void UdpVideoStreamClient2::receiveStream(Packet *pk)
{
    if (reintentTimer->isScheduled())
        cancelEvent(reintentTimer);
    if (timeOutMsg->isScheduled())
        cancelEvent(timeOutMsg);

    recieved = true;

    if (timeOut > 0 && par("multipleRequest").boolValue())
        scheduleAt(simTime()+timeOut,timeOutMsg); // only if multiple request is active


    if (simTime() - pk->getCreationTime() > limitDelay)
    {
        delete pk;
        return;
    }

    auto chunk = pk->peekAtFront<Chunk>();

    auto vpkt =  dynamicPtrCast<const VideoPacket> (chunk);

    if (vpkt) {
        do {
            auto vpktAux =  pk->popAtFront<VideoPacket>();
            if (vpktAux->getSeqNum() > lastSeqNum)
                lastSeqNum = vpktAux->getSeqNum();
            else  {
                delete pk;
                return;
            }

            switch(vpktAux->getType()) {
                case 'P':
                    numPframes++;
                    totalBytesP += vpktAux->getFrameSize().get();
                    break;
                case 'B':
                    numBframes++;
                    totalBytesB += vpktAux->getFrameSize().get();
                    break;
                case 'I':
                    numIframes++;
                    totalBytesI += vpktAux->getFrameSize().get();
                    break;
            }
        } while(pk->getBitLength() > 0);
    }

    numRecPackets++;
    EV << "Video stream packet: " << UdpSocket::getReceivedPacketInfo(pk) << endl;
    emit(rcvdPkSignal, pk);

    delete pk;
}

void UdpVideoStreamClient2::timeOutData()
{
    double reint = par("reintent").intValue();
    if (reint > 0)
        scheduleAt(simTime()+par("reintent").intValue(),reintentTimer);
}

void UdpVideoStreamClient2::handleStartOperation(LifecycleOperation *operation)
{
    simtime_t startTimePar = par("startTime");
    simtime_t startTime = std::max(startTimePar, simTime());
    scheduleAt(startTime, selfMsg);
}

void UdpVideoStreamClient2::handleStopOperation(LifecycleOperation *operation)
{
    cancelEvent(selfMsg);
    cancelEvent(reintentTimer);
    cancelEvent(timeOutMsg);
    //TODO if(socket.isOpened()) socket.close();
    socket.close();
    delayActiveOperationFinish(par("stopOperationTimeout"));
}

void UdpVideoStreamClient2::handleCrashOperation(LifecycleOperation *operation)
{
    cancelEvent(selfMsg);
    cancelEvent(reintentTimer);
    cancelEvent(timeOutMsg);
    if (operation->getRootModule() != getContainingNode(this))     // closes socket when the application crashed only
        socket.destroy();    //TODO  in real operating systems, program crash detected by OS and OS closes sockets of crashed programs.
}


}

