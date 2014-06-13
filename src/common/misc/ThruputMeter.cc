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


#include "ThruputMeter.h"

Define_Module(ThruputMeter);


void ThruputMeter::initialize()
{
    startTime = par("startTime");
    long _batchSize = par("batchSize");
    if ((_batchSize < 0) || (((long)(unsigned int)_batchSize) != _batchSize))
        throw cRuntimeError("Invalid 'batchSize=%ld' parameter at '%s' module", _batchSize, getFullPath().c_str());
    batchSize = (unsigned int)_batchSize;
    maxInterval = par("maxInterval");

    numPackets = numBits = 0;
    intvlStartTime = intvlLastPkTime = 0;
    intvlNumPackets = intvlNumBits = 0;

    WATCH(numPackets);
    WATCH(numBits);
    WATCH(intvlStartTime);
    WATCH(intvlNumPackets);
    WATCH(intvlNumBits);

    bitpersecVector.setName("thruput (bit/sec)");
    pkpersecVector.setName("packet/sec");
}

void ThruputMeter::handleMessage(cMessage *msg)
{
    updateStats(simTime(), PK(msg)->getBitLength());
    send(msg, "out");
}

void ThruputMeter::updateStats(simtime_t now, unsigned long bits)
{
    numPackets++;
    numBits += bits;

    // packet should be counted to new interval
    if (intvlNumPackets >= batchSize || now-intvlStartTime >= maxInterval)
        beginNewInterval(now);

    intvlNumPackets++;
    intvlNumBits += bits;
    intvlLastPkTime = now;
}

void ThruputMeter::beginNewInterval(simtime_t now)
{
    simtime_t duration = now - intvlStartTime;

    // record measurements
    double bitpersec = intvlNumBits/duration.dbl();
    double pkpersec = intvlNumPackets/duration.dbl();

    bitpersecVector.recordWithTimestamp(intvlStartTime, bitpersec);
    pkpersecVector.recordWithTimestamp(intvlStartTime, pkpersec);

    // restart counters
    intvlStartTime = now;  // FIXME this should be *beginning* of tx of this packet, not end!
    intvlNumPackets = intvlNumBits = 0;
}

void ThruputMeter::finish()
{
    simtime_t duration = simTime() - startTime;

    recordScalar("duration", duration);
    recordScalar("total packets", numPackets);
    recordScalar("total bits", numBits);

    recordScalar("avg throughput (bit/s)", numBits/duration.dbl());
    recordScalar("avg packets/s", numPackets/duration.dbl());
}


