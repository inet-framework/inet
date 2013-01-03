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
    UDPVideoStreamCli::initialize();

    // initialize module parameters
    clockFrequency = par("clockFrequency").doubleValue();

    // initialize status variables
    prevTimestampReceived = false;

    // initialize statistics
    fragmentStartSignal = registerSignal("fragmentStart");
    interArrivalTimeSignal = registerSignal("interArrivalTime");
    interDepartureTimeSignal = registerSignal("interDepartureTime");
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
//    double dbg_time = simTime().dbl();
//    uint64_t dbg_raw_time = simTime().raw();
//    uint64_t dbg_time_scale = simTime().getScale();
//    uint64_t dbg_ctrValue = uint64_t(clockFrequency)*simTime().raw()/simTime().getScale();
//    uint32_t dbg_wrappedCtrValue = uint32_t(dbg_ctrValue%0x100000000LL);
//    // DEBUG

    emit(fragmentStartSignal, int(((UDPVideoStreamPacket *)msg)->getFragmentStart()));

    if (prevTimestampReceived == false)
    { // not initialized yet
         prevArrivalTime = uint32_t(uint64_t(clockFrequency*simTime().dbl())%0x100000000LL);   // value of a latched counter driven by a local clock
//        prevArrivalTime = uint32_t((uint64_t(clockFrequency)*simTime().raw()/simTime().getScale())%0x100000000LL);    // value of a latched counter driven by a local clock
        prevTimestamp = ((UDPVideoStreamPacket *)msg)->getTimestamp();
        prevTimestampReceived = true;
    }
    else
    {
        uint32_t currArrivalTime = uint32_t(uint64_t(clockFrequency*simTime().dbl())%0x100000000LL);
//        uint32_t currArrivalTime = uint32_t((uint64_t(clockFrequency)*simTime().raw()/simTime().getScale())%0x100000000LL);    // value of a latched counter driven by a local clock
        uint32_t currTimestamp = ((UDPVideoStreamPacket *)msg)->getTimestamp();

        int64_t interArrivalTime = int64_t(currArrivalTime) - int64_t(prevArrivalTime);
        if (currArrivalTime <= prevArrivalTime)
        {   // handling wrap around
            interArrivalTime += 0x100000000LL;
        }
        int64_t interDepartureTime = int64_t(currTimestamp) - int64_t(prevTimestamp);
        if (currTimestamp <= prevTimestamp)
        {   // handling wrap around
            interDepartureTime += 0x100000000LL;
        }
        emit(interArrivalTimeSignal, double(interArrivalTime));
        emit(interDepartureTimeSignal, double(interDepartureTime));
        emit(measuredClockRatioSignal, double(interArrivalTime)/double(interDepartureTime));

        prevArrivalTime = currArrivalTime;
        prevTimestamp = currTimestamp;
    }

    UDPVideoStreamCli::receiveStream(msg);  // 'msg' is deleted in this function
}
