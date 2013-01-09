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


#define MODULO_32 0x100000000LL     // modulo for 32-bit counter
#define MODULO_48 0x1000000000000LL // module for 48-bit counter


Define_Module(UDPVideoStreamCliWithSCFR);


void UDPVideoStreamCliWithSCFR::initialize()
{
    // initialize module parameters
    simtime_t startTime = par("startTime");
    clockFrequency = par("clockFrequency").doubleValue();

    // initialize status variables
    prevTsReceivedAperiodic = false;
    prevTsReceivedPeriodic = false;

    // initialize statistics: common
    fragmentStartSignal = registerSignal("fragmentStart");

    // initialize statistics: aperiodic
    eedAperiodicSignal = registerSignal("endToEndDelayAperiodic");
    iatAperiodicSignal = registerSignal("interArrivalTimeAperiodic");
    idtAperiodicSignal = registerSignal("interDepartureTimeAperiodic");
    cfrAperiodicSignal = registerSignal("clockFrequencyRatioAperiodic");

    // initialize statistics: periodic
    eedPeriodicSignal = registerSignal("endToEndDelayPeriodic");
    iatPeriodicSignal = registerSignal("interArrivalTimePeriodic");
    idtPeriodicSignal = registerSignal("interDepartureTimePeriodic");
    cfrPeriodicSignal = registerSignal("clockFrequencyRatioPeriodic");

    // schedule the start of video streaming
    if (startTime>=0)
        scheduleAt(startTime, new cMessage("UDPVideoStreamStart"));
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

//    // DEBUG
//    double dbg_time = simTime().dbl();
//    uint64_t dbg_raw_time = simTime().raw();
//    uint64_t dbg_time_scale = simTime().getScale();
//    uint64_t dbg_ctrValue = uint64_t(clockFrequency)*simTime().raw()/simTime().getScale();
//    uint64_t dbg_wrappedCtrValue = dbg_ctrValue%MODULO_48;
//    // DEBUG

    bool fragmentStart = ((UDPVideoStreamPacket *)msg)->getFragmentStart();
    emit(fragmentStartSignal, int(fragmentStart));

    // processing for all packets (i.e., aperiodic case)
    emit(eedAperiodicSignal, simTime() - msg->getCreationTime());
    if (prevTsReceivedAperiodic == false)
    { // not initialized yet
         prevAtAperiodic = uint64_t(clockFrequency*simTime().dbl())%MODULO_48;  // latched value of a 48-bit counter driven by a local clock
        prevTsAperiodic = ((UDPVideoStreamPacket *)msg)->getTimestamp();
        prevTsReceivedAperiodic = true;
    }
    else
    {
        uint64_t currArrivalTime = uint64_t(clockFrequency*simTime().dbl())%MODULO_48;
        uint32_t currTimestamp = ((UDPVideoStreamPacket *)msg)->getTimestamp();

        int64_t interArrivalTime = currArrivalTime - prevAtAperiodic;
        if (currArrivalTime <= prevAtAperiodic)
        {   // handling wrap around
            interArrivalTime += MODULO_48;
        }
        int64_t interDepartureTime = int64_t(currTimestamp) - int64_t(prevTsAperiodic);
        if (currTimestamp <= prevTsAperiodic)
        {   // handling wrap around
            interDepartureTime += MODULO_32;
        }
        emit(iatAperiodicSignal, double(interArrivalTime));
        emit(idtAperiodicSignal, double(interDepartureTime));
        emit(cfrAperiodicSignal, double(interArrivalTime)/double(interDepartureTime));

        prevAtAperiodic = currArrivalTime;
        prevTsAperiodic = currTimestamp;
    }

    // processing for the first packets of frames (i.e., periodic case)
    if (fragmentStart == true)
    {
        emit(eedPeriodicSignal, simTime() - msg->getCreationTime());
        if (prevTsReceivedPeriodic == false)
        { // not initialized yet
            prevAtPeriodic = uint64_t(clockFrequency*simTime().dbl())%MODULO_48;    // latched value of a 48-bit counter driven by a local clock
            prevTsPeriodic = ((UDPVideoStreamPacket *)msg)->getTimestamp();
            prevTsReceivedPeriodic = true;
        }
        else
        {
            uint64_t currArrivalTime = uint64_t(clockFrequency*simTime().dbl())%MODULO_48;
            uint32_t currTimestamp = ((UDPVideoStreamPacket *)msg)->getTimestamp();

            int64_t interArrivalTime = currArrivalTime - prevAtPeriodic;
            if (currArrivalTime <= prevAtPeriodic)
            {   // handling wrap around
                interArrivalTime += MODULO_48;
            }
            int64_t interDepartureTime = int64_t(currTimestamp) - int64_t(prevTsPeriodic);
            if (currTimestamp <= prevTsPeriodic)
            {   // handling wrap around
                interDepartureTime += MODULO_32;
            }
            emit(iatPeriodicSignal, double(interArrivalTime));
            emit(idtPeriodicSignal, double(interDepartureTime));
            emit(cfrPeriodicSignal, double(interArrivalTime)/double(interDepartureTime));

            prevAtPeriodic = currArrivalTime;
            prevTsPeriodic = currTimestamp;
        }
    }

    delete msg;
}
