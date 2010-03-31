//
// Copyright (C) 2010 Kyeong Soo (Joseph) Kim
// Copyright (C) 2005 Andras Varga
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

#include "UDPVideoStreamCliWithTrace.h"
#include "IPAddressResolver.h"


Define_Module(UDPVideoStreamCliWithTrace);


//void UDPVideoStreamCliWithTrace::initialize()
//{
//    eed.setName("video stream eed");
//    simtime_t startTime = par("startTime");
//
//    if (startTime>=0)
//        scheduleAt(startTime, new cMessage("UDPVideoStreamStart"));
//}

//void UDPVideoStreamCliWithTrace::finish()
//{
//}

//void UDPVideoStreamCliWithTrace::handleMessage(cMessage* msg)
//{
//    if (msg->isSelfMessage())
//    {
//        delete msg;
//        requestStream();
//    }
//    else
//    {
//        receiveStream(PK(msg));
//    }
//}

//void UDPVideoStreamCliWithTrace::requestStream()
//{
//    int svrPort = par("serverPort");
//    int localPort = par("localPort");
//    const char *address = par("serverAddress");
//    IPvXAddress svrAddr = IPAddressResolver().resolve(address);
//    if (svrAddr.isUnspecified())
//    {
//        EV << "Server address is unspecified, skip sending video stream request\n";
//        return;
//    }
//
//    EV << "Requesting video stream from " << svrAddr << ":" << svrPort << "\n";
//
//    bindToPort(localPort);
//
//    cPacket *msg = new cPacket("VideoStrmReq");
//    sendToUDP(msg, localPort, svrAddr, svrPort);
//}

void UDPVideoStreamCliWithTrace::receiveStream(cPacket *msg)
{
    EV << "Video stream packet:\n";
    printPacket(msg);
    eed.record(simTime() - msg->getCreationTime());

    // TODO: Do packet loss processing based on sequence numbers
    // -- Initialize simple timer modeling playout buffer (e.g., T=5s).
    // -- If interarrival time between two packets are within frame period, no change (?)
    // -- Otherwise, decrease T accordingly (e.g., T - (IA - frame period)?);
    // -- if T becomes negative, treat the packet as lost one and reset T.

    delete msg;
}

