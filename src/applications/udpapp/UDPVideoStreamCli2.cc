//
// Copyright (C) 2005 Andras Varga
// Copyright (C) 2012 Alfonso Ariza
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

#include "UDPVideoStreamCli2.h"

#include "UDPControlInfo_m.h"
#include "AddressResolver.h"
#include "VideoPacket_m.h"


Define_Module(UDPVideoStreamCli2);

simsignal_t UDPVideoStreamCli2::rcvdPkSignal = registerSignal("rcvdPk");

UDPVideoStreamCli2::UDPVideoStreamCli2()
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

UDPVideoStreamCli2::~UDPVideoStreamCli2()
{
    cancelAndDelete(reintentTimer);
    cancelAndDelete(timeOutMsg);
    cancelAndDelete(selfMsg);
}

void UDPVideoStreamCli2::initialize(int stage)
{
    ApplicationBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL)
    {
        // statistics
        timeOut  = par("timeOut");
        limitDelay = par("limitDelay");
    }
}

void UDPVideoStreamCli2::finish()
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

void UDPVideoStreamCli2::handleMessageWhenUp(cMessage* msg)
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
    else if (msg->getKind() == UDP_I_DATA)
    {
        // process incoming packet
        receiveStream(PK(msg));
    }
    else if (msg->getKind() == UDP_I_ERROR)
    {
        EV << "Ignoring UDP error report\n";
        delete msg;
    }
    else
    {
        error("Unrecognized message (%s)%s", msg->getClassName(), msg->getName());
    }
}

void UDPVideoStreamCli2::requestStream()
{

    if (recieved && !par("multipleRequest").boolValue())
        return;
    int svrPort = par("serverPort");
    int localPort = par("localPort");
    const char *address = par("serverAddress");
    Address svrAddr = AddressResolver().resolve(address);

    if (svrAddr.isUnspecified())
    {
        EV << "Server address is unspecified, skip sending video stream request\n";
        return;
    }

    EV << "Requesting video stream from " << svrAddr << ":" << svrPort << "\n";

    if (!socketOpened)
    {
        socket.setOutputGate(gate("udpOut"));
        socket.bind(localPort);
        socketOpened = true;
    }

    lastSeqNum = -1;
    cPacket *pk = new cPacket("VideoStrmReq");
    socket.sendTo(pk, svrAddr, svrPort);
    double reint = par("reintent").longValue();
    if (reint > 0)
        scheduleAt(simTime()+par("reintent").longValue(),reintentTimer);
}

void UDPVideoStreamCli2::receiveStream(cPacket *pk)
{
    if (reintentTimer->isScheduled())
        cancelEvent(reintentTimer);
    if (timeOutMsg->isScheduled())
        cancelEvent(timeOutMsg);
    if (timeOut > 0)
        scheduleAt(simTime()+timeOut,timeOutMsg);

    recieved = true;

    if (simTime() - pk->getCreationTime() > limitDelay)
    {
        delete pk;
        return;
    }

    VideoPacket *vpkt = dynamic_cast<VideoPacket*> (pk->getEncapsulatedPacket());

    if (vpkt)
    {
        do
        {
            if (vpkt->getSeqNum() > lastSeqNum)
                lastSeqNum = vpkt->getSeqNum();
            else
            {
                delete pk;
                return;
            }

            switch(vpkt->getType())
            {
                case 'P':
                    numPframes++;
                    totalBytesP += vpkt->getFrameSize();
                    break;
                case 'B':
                    numBframes++;
                    totalBytesB += vpkt->getFrameSize();
                    break;
                case 'I':
                    numIframes++;
                    totalBytesI += vpkt->getFrameSize();
                    break;
            }
            vpkt = dynamic_cast<VideoPacket*> (vpkt->getEncapsulatedPacket());
        } while(vpkt);
    }

    numRecPackets++;
    EV << "Video stream packet: " << UDPSocket::getReceivedPacketInfo(pk) << endl;
    emit(rcvdPkSignal, pk);

    delete pk;
}

void UDPVideoStreamCli2::timeOutData()
{
    double reint = par("reintent").longValue();
    if (reint > 0)
        scheduleAt(simTime()+par("reintent").longValue(),reintentTimer);
}

bool UDPVideoStreamCli2::handleNodeStart(IDoneCallback *doneCallback)
{
    simtime_t startTimePar = par("startTime");
    simtime_t startTime = std::max(startTimePar, simTime());
    scheduleAt(startTime, selfMsg);
    return true;
}

bool UDPVideoStreamCli2::handleNodeShutdown(IDoneCallback *doneCallback)
{
    cancelEvent(selfMsg);
    cancelEvent(reintentTimer);
    cancelEvent(timeOutMsg);
    //TODO if(socket.isOpened()) socket.close();
    return true;
}

void UDPVideoStreamCli2::handleNodeCrash()
{
    cancelEvent(selfMsg);
    cancelEvent(reintentTimer);
    cancelEvent(timeOutMsg);
}


