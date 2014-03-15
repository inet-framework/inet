///
/// @file   PerformanceMeter.cc
/// @author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
/// @date   February/20/2014
///
/// @brief  Implements 'PerformanceMeter' class for measuring frame/packet end-to-end
///         delay and throughput.
///
/// @remarks Copyright (C) 2014 Kyeong Soo (Joseph) Kim. All rights reserved.
///
/// @remarks This software is written and distributed under the GNU General
///          Public License Version 2 (http://www.gnu.org/licenses/gpl-2.0.html).
///          You must not remove this notice, or any other, from this software.
///


#include "PerformanceMeter.h"

// Register modules.
Define_Module(PerformanceMeter);

PerformanceMeter::PerformanceMeter()
{
}

PerformanceMeter::~PerformanceMeter()
{
    cancelAndDelete(measurementTimer);
}

void PerformanceMeter::initialize()
{
    startTime = par("startTime");
    measurementInterval = par("measurementInterval");

    sumPacketDelays = 0.0;
//    intvSumPacketDelays = 0.0;
    numPackets = numBits = 0;
    intvlNumPackets = intvlNumBits = 0;

    WATCH(numPackets);
    WATCH(numBits);
    WATCH(intvlNumPackets);
    WATCH(intvlNumBits);

    packetDelaySignal = registerSignal("packetDelay");
    bitThruputSignal = registerSignal("bitThruput");
    packetThruputSignal = registerSignal("packetThruput");
    // bitpersecVector.setName("thruput (bit/sec)");
    // pkpersecVector.setName("packet/sec");

    measurementTimer = new cMessage("measurementTimer", 0);
    scheduleAt(startTime, measurementTimer);
}

void PerformanceMeter::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage())
    {   // measurement interval
        if (measurementStarted == false)
        {
            measurementStarted = true;
        }

        // record measurements
//        double packetDelay = intvSumPacketDelays/intvlNumPackets;
        double bitThruput = intvlNumBits/measurementInterval.dbl();
        double packetThruput = intvlNumPackets/measurementInterval.dbl();

        // emit statistics signals
//        emit(packetDelaySignal, packetDelay);
        emit(bitThruputSignal, bitThruput);
        emit(packetThruputSignal, packetThruput);
        // bitpersecVector.record(bitpersec);
        // pkpersecVector.record(pkpersec);

        // restart interval counters
        intvlNumPackets = intvlNumBits = 0;

        // schedule new interval
        scheduleAt(simTime()+measurementInterval, measurementTimer);
    }
    else
    {   // packet arrival
        if (measurementStarted == true)
        {
            // packet delay
            double packetDelay = SIMTIME_DBL(simTime() - PK(msg)->getCreationTime());
            emit(packetDelaySignal, packetDelay);
            sumPacketDelays += packetDelay;

            // packet throughput
            int bits = PK(msg)->getBitLength();
            numPackets++;
            numBits += bits;
            intvlNumPackets++;
            intvlNumBits += bits;
        }
        send(msg, "out");
    }
}

void PerformanceMeter::finish()
{
    simtime_t duration = simTime() - startTime;

    recordScalar("duration", duration);
    recordScalar("total packets", numPackets);
    recordScalar("total bits", numBits);

    recordScalar("avg packet delay (s)", sumPacketDelays/numPackets);
    recordScalar("avg throughput (bit/s)", numBits/duration.dbl());
    recordScalar("avg throughput (packet/s)", numPackets/duration.dbl());
}
