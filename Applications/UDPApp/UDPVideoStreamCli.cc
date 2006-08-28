//
// Copyright (C) 2005 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//


//
// based on the video streaming app of the similar name by Johnny Lai
//

#include "UDPVideoStreamCli.h"
#include "IPAddressResolver.h"


Define_Module(UDPVideoStreamCli);


void UDPVideoStreamCli::initialize()
{
    eed.setName("video stream eed");
    double startTime = par("startTime");

    if (startTime>=0)
        scheduleAt(startTime, new cMessage("UDPVideoStreamStart"));
}

void UDPVideoStreamCli::finish()
{
}

void UDPVideoStreamCli::handleMessage(cMessage* msg)
{
    if (msg->isSelfMessage())
    {
        delete msg;
        requestStream();
    }
    else
    {
        receiveStream(msg);
    }
}

void UDPVideoStreamCli::requestStream()
{
    int svrPort = par("serverPort");
    int localPort = par("localPort");
    const char *address = par("serverAddress");
    IPvXAddress svrAddr = IPAddressResolver().resolve(address);

    EV << "Requesting video stream from " << svrAddr << ":" << svrPort << "\n";

    bindToPort(localPort);

    cMessage *msg = new cMessage("VideoStrmReq");
    sendToUDP(msg, localPort, svrAddr, svrPort);
}

void UDPVideoStreamCli::receiveStream(cMessage* msg)
{
    EV << "Video stream packet:\n";
    printPacket(msg);
    eed.record(simTime() - msg->creationTime());
    delete msg;
}

