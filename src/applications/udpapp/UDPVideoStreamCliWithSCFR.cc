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

#ifndef UINT32_MAX
#define UINT32_MAX (0xffffffff)
#endif

#include "UDPVideoStreamCliWithSCFR.h"
#include "IPAddressResolver.h"


Define_Module(UDPVideoStreamCliWithSCFR);


void UDPVideoStreamCliWithSCFR::initialize()
{
    UDPVideoStreamCli::initialize();

    // initialize module parameters
    clockFrequency = par("clockFrequency").doubleValue();

    // initialize status variables
    prevTimestampReceived = false;

    // initialize statistics
    measuredClockRatioSignal = registerSignal("measuredClockRatio");
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
//    EV << "Received " << msg->getName() << ", lifetime: " << lifetime << "s" << endl;

//    // DEBUG
//    double currTime = simTime().dbl();
//    uint32_t counterValue = uint32_t(clockFrequency * currTime);
//    uint64_t max_tmp = UINT32_MAX + 1LL;
//    double double_tmp = fmod(clockFrequency*currTime, UINT32_MAX+1LL);
//    uint32_t int_tmp = uint32_t(double_tmp);
//    // DEBUG

    if (prevTimestampReceived == false)
    { // not initialized yet
        prevArrivalTime = uint32_t(fmod(clockFrequency*simTime().dbl(), UINT32_MAX+1LL));   // value of a latched counter driven by a local clock
        prevTimestamp = ((UDPVideoStreamPacket *)msg)->getTimestamp();
        prevTimestampReceived = true;
    }
    else
    {
        uint32_t currArrivalTime = uint32_t(fmod(clockFrequency*simTime().dbl(), UINT32_MAX+1LL));
        uint32_t currTimestamp = ((UDPVideoStreamPacket *)msg)->getTimestamp();

        int64_t arrivalTimeDifference = int64_t(currArrivalTime) - int64_t(prevArrivalTime);
        if (currArrivalTime <= prevArrivalTime)
        {   // handling wrap around
            arrivalTimeDifference += UINT32_MAX + 1LL;
        }
        int64_t timestampDifference = int64_t(currTimestamp) - int64_t(prevTimestamp);
        if (currTimestamp <= prevTimestamp)
        {   // handling wrap around
            timestampDifference += UINT32_MAX + 1LL;
        }
        emit(measuredClockRatioSignal, double(arrivalTimeDifference)/double(timestampDifference));

        prevArrivalTime = currArrivalTime;
        prevTimestamp = currTimestamp;
    }

    UDPVideoStreamCli::receiveStream(msg);  // 'msg' is deleted in this function
}
