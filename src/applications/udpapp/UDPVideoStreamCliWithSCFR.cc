//
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

#include "UDPVideoStreamCliWithSCFR.h"
#include "IPAddressResolver.h"


Define_Module(UDPVideoStreamCliWithSCFR);


void UDPVideoStreamCliWithSCFR::initialize()
{
    // from the base class initialization
    simtime_t startTime = par("startTime");
    if (startTime>=0)
        scheduleAt(startTime, new cMessage("UDPVideoStreamStart"));

    // initialize module parameters
    clockFrequency = par("clockFrequency").doubleValue();

    // initialize status variables
    prevArrivalTime = -1L;
    prevTimestamp = -1L;

    // initialize statistics
    estimatedClockRatioSignal = registerSignal("estimated ratio of clock frequencies between source and receiver");
}

void UDPVideoStreamCliWithSCFR::handleMessage(cMessage* msg)
{
    if (msg->isSelfMessage())
    {
        delete msg;
        requestStream();
    }
    else
    {
        receiveStream(PK(msg));
    }
}

void UDPVideoStreamCliWithSCFR::receiveStream(cPacket *msg)
{
    EV << "Video stream packet:\n";
    printPacket(msg);

//    EV << "Received " << msg->getName() << ", lifetime: " << lifetime << "s" << endl;

    if (prevArrivalTime < 0L)
    { // not initialized yet
        prevArrivalTime = uint32_t(clockFrequency * simTime().dbl()); // value of latched counter driven by a local clock
        prevTimestamp = ((UDPVideoStreamPacket *) msg)->getTimestamp();
    }
    else
    {
        uint32_t currArrivalTime = uint32_t(fmod(clockFrequency*simTime().dbl(), UINT32_MAX+1));
        uint32_t currTimestamp = ((UDPVideoStreamPacket *)msg)->getTimestamp();

        double arrivalTimeDifference = currArrivalTime - prevArrivalTime;
        if (currArrivalTime <= prevArrivalTime)
        {   // handling wrap around
            arrivalTimeDifference += UINT32_MAX + 1.0;
        }
        double timestampDifference = currTimestamp - prevTimestamp;
        if (currTimestamp <= prevTimestamp)
        {   // handling wrap around
            timestampDifference += UINT32_MAX + 1.0;
        }
        emit(estimatedClockRatioSignal, arrivalTimeDifference / timestampDifference);

        prevArrivalTime = currArrivalTime;
        prevTimestamp = currTimestamp;
    }

    delete msg;
}
