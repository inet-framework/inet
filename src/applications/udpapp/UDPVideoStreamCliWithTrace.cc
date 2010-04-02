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

///
/// @file   UDPVideoStreamCliWithTrace.cc
/// @author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
/// @date   2010-04-02
///
/// @brief  Implements UDPVideoStreamCliWithTrace class.
///
/// @note
/// This file implements UDPVideoStreamCliWithTrace, modeling a video
/// streaming client based on trace files from ASU video trace library [1].
///
/// @par References:
/// <ol>
///	<li><a href="http://trace.eas.asu.edu/">Video trace library, Arizona State University</a>
/// </li>
/// </ol>
///

#include "UDPVideoStreamCliWithTrace.h"
#include "IPAddressResolver.h"


Define_Module(UDPVideoStreamCliWithTrace);


void UDPVideoStreamCliWithTrace::initialize()
{
//    eed.setName("video stream eed");
//    simtime_t startTime = par("startTime");
//
//    if (startTime>=0)
//        scheduleAt(startTime, new cMessage("UDPVideoStreamStart"));

	UDPVideoStreamCli::initialize();

	// initialize module parameters
	startupDelay = par(startupDelay).doubleValue();	///< unit is second

	// initialize statistics
	numPktRcvd = 0;
	numPktLost = 0;

	isFirstPacket = true;
}

void UDPVideoStreamCliWithTrace::finish()
{
	UDPVideoStreamCli::finish();

	recordScalar("packets received", numPktRcvd);
	recordScalar("packets lost", numPktLost);
}

void UDPVideoStreamCliWithTrace::handleMessage(cMessage* msg)
{
	if (msg->isSelfMessage())
	{
		delete msg;
		requestStream();
	}
	else
	{
		receiveStream(check_and_cast<UDPVideoStreamPacket *>(msg));
	}
}

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

void UDPVideoStreamCliWithTrace::receiveStream(UDPVideoStreamPacket *pkt)
{
    EV << "Video stream packet:\n";
    printPacket(PK(pkt));

    // record statistics
    eed.record(simTime() - pkt->getCreationTime());

    numPktRcvd++;

    unsigned short seqNumber = pkt->getSequenceNumber();
    if (isFirstPacket)
    {
    	isFirstPacket = false;
    }
    else
    {
        // TODO: Implement advanced packet and frame loss processing later:
        // -- Initialize simple timer modeling playout buffer (e.g., T=5s).
        // -- If interarrival time between two packets are within frame period, no change (?)
        // -- Otherwise, decrease T accordingly (e.g., T - (IA - frame period)?);
        // -- if T becomes negative, treat the packet as lost one and reset T.

    	if ( seqNumber != ((prevSequenceNumber + 1) % 65536) )
    	{
    		// previous packet(s) lost before the current one

    		int currnetNumPktLost = seqNumber > prevSequenceNumber ? seqNumber - prevSequenceNumber : seqNumber + 65536 - prevSequenceNumber;
    		numPktLost += currnetNumPktLost;
//    		if (seqNumber > prevSequenceNumber)
//    		{
//    			numPktLost += seqNumber - prevSequenceNumber;
//    		}
//    		else
//    		{
//    			numPktLost += seqNumber + 65536 - prevSequenceNumber;
//    		}

    		EV << currnetNumPktLost << " video stream packet(s) lost" << endl;
    	}
    }
    prevSequenceNumber = seqNumber;

    delete pkt;
}
