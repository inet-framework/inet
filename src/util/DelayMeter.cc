///
/// @file   DelayMeter.cc
/// @author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
/// @date   Mar/1/2012
///
/// @brief  Implements 'DelayMeter' class for measuring frame/packet end-to-end delay.
///
/// @remarks Copyright (C) 2012 Kyeong Soo (Joseph) Kim. All rights reserved.
///
/// @remarks This software is written and distributed under the GNU General
///          Public License Version 2 (http://www.gnu.org/licenses/gpl-2.0.html).
///          You must not remove this notice, or any other, from this software.
///


#include "DelayMeter.h"

// Register modules.
Define_Module(DelayMeter);

void DelayMeter::initialize()
{
    packetDelaySignal = registerSignal("packetDelay");
    numPackets = 0;
    sumPacketDelays = 0.0;
    WATCH(numPackets);
}

void DelayMeter::handleMessage(cMessage *msg)
{
    if (simTime() >= simulation.getWarmupPeriod())
    {
        double packetDelay = SIMTIME_DBL(simTime() - ((cPacket *) msg)->getCreationTime());

        // emit statistics signals
        emit(packetDelaySignal, packetDelay);

        // update statistics
        numPackets++;
        sumPacketDelays += packetDelay;
    }

    send(msg, "out");
}

void DelayMeter::finish()
{
    // record session statistics
    if (numPackets > 0)
    {
        double avgPacketDelay = sumPacketDelays/double(numPackets);
        recordScalar("number of packets measured", numPackets);
        recordScalar("average packet delay [s]", avgPacketDelay);
    }
}
