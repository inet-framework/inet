//
// Copyright (C) 2013 Kyeong Soo (Joseph) Kim
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


#include "ThruputMeter2.h"

Define_Module(ThruputMeter2);

ThruputMeter2::ThruputMeter2()
{
}

ThruputMeter2::~ThruputMeter2()
{
    cancelAndDelete(measurementTimer);
}

void ThruputMeter2::initialize()
{
    startTime = par("startTime");
    measurementInterval = par("measurementInterval");

    numPackets = numBits = 0;
    intvlNumPackets = intvlNumBits = 0;

    WATCH(numPackets);
    WATCH(numBits);
    WATCH(intvlNumPackets);
    WATCH(intvlNumBits);

    bitpersecVector.setName("thruput (bit/sec)");
    pkpersecVector.setName("packet/sec");

    measurementTimer = new cMessage("measurementTimer", 0);
    scheduleAt(startTime, measurementTimer);
}

void ThruputMeter2::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage())
    {   // measurement interval
        if (measurementStarted == false)
        {
            measurementStarted = true;
        }

        // record measurements
        double bitpersec = intvlNumBits/measurementInterval.dbl();
        double pkpersec = intvlNumPackets/measurementInterval.dbl();

        bitpersecVector.record(bitpersec);
        pkpersecVector.record(pkpersec);

        // restart interval counters
        intvlNumPackets = intvlNumBits = 0;

        // schedule new interval
        scheduleAt(simTime()+measurementInterval, measurementTimer);
    }
    else
    {   // packet arrival
        if (measurementStarted == true)
        {
            int bits = PK(msg)->getBitLength();
            numPackets++;
            numBits += bits;
            intvlNumPackets++;
            intvlNumBits += bits;
        }
        send(msg, "out");
    }
}

void ThruputMeter2::finish()
{
    simtime_t duration = simTime() - startTime;

    recordScalar("duration", duration);
    recordScalar("total packets", numPackets);
    recordScalar("total bits", numBits);

    recordScalar("avg throughput (bit/s)", numBits/duration.dbl());
    recordScalar("avg packets/s", numPackets/duration.dbl());
}
