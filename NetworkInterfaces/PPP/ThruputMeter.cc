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


#include "ThruputMeter.h"

Define_Module(ThruputMeter);


void ThruputMeter::initialize()
{
    starttime = par("startTime");
    batchsize = par("batchSize");
    maxinterval = par("maxInterval");

    numpackets = numbits = 0;
    intvl_starttime = intvl_lastpktime = 0;
    intvl_numpackets = intvl_numbits = 0;

    WATCH(numpackets);
    WATCH(numbits);
    WATCH(intvl_starttime);
    WATCH(intvl_numpackets);
    WATCH(intvl_numbits);

    bitpersecVector.setName("thruput (bit/sec)");
    pkpersecVector.setName("packet/sec");
}

void ThruputMeter::handleMessage(cMessage *msg)
{
    updateStats(simTime(), msg->length());
    send(msg, "out");
}

void ThruputMeter::updateStats(simtime_t now, unsigned long bits)
{
    numpackets++;
    numbits += bits;

    // packet should be counted to new interval
    if (intvl_numpackets >= batchsize || now-intvl_starttime >= maxinterval)
        beginNewInterval(now);

    intvl_numpackets++;
    intvl_numbits += bits;
    intvl_lastpktime = now;
}

void ThruputMeter::beginNewInterval(simtime_t now)
{
    simtime_t duration = now - intvl_starttime;

    // record measurements
    double bitpersec = intvl_numbits/duration;
    double pkpersec = intvl_numpackets/duration;

//FIXME introduce recordAt() into omnetpp!!!
#define recordAt(t,d)   record(d)
    bitpersecVector.recordAt(intvl_starttime, bitpersec);
    pkpersecVector.recordAt(intvl_starttime, pkpersec);
#undef recordAt

    // restart counters
    intvl_starttime = now;  // FIXME this should be *beginning* of tx of this packet, not end!
    intvl_numpackets = intvl_numbits = 0;
}

void ThruputMeter::finish()
{
    simtime_t duration = simTime() - starttime;

    recordScalar("duration", duration);
    recordScalar("total packets", numpackets);
    recordScalar("total bits", numbits);

    recordScalar("avg throughput (bit/s)", numbits/duration);
    recordScalar("avg packets/s", numpackets/duration);
}


