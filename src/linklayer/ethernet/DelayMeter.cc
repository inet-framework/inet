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
    frameDelaySignal = registerSignal("frameDelay");
    numFrames = 0;
    WATCH(numFrames);
}

void DelayMeter::handleMessage(cMessage *msg)
{
    // emit statistics signals
    emit(frameDelaySignal, SIMTIME_DBL(simTime() - ((cPacket *) msg)->getCreationTime()));

    // update statistics
    numFrames++;

    send(msg, "out");
}

void DelayMeter::finish()
{
    recordScalar("number of frames measured", numFrames);
}


