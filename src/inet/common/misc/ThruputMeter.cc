//
// Copyright (C) 2005 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/misc/ThruputMeter.h"

namespace inet {

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
    if (intvlNumPackets >= batchSize || now - intvlStartTime >= maxInterval)
        beginNewInterval(now);

    intvlNumPackets++;
    intvlNumBits += bits;
    intvlLastPkTime = now;
}

void ThruputMeter::beginNewInterval(simtime_t now)
{
    simtime_t duration = now - intvlStartTime;

    // record measurements
    double bitpersec = intvlNumBits / duration.dbl();
    double pkpersec = intvlNumPackets / duration.dbl();

    bitpersecVector.recordWithTimestamp(intvlStartTime, bitpersec);
    pkpersecVector.recordWithTimestamp(intvlStartTime, pkpersec);

    // restart counters
    intvlStartTime = now; // FIXME this should be *beginning* of tx of this packet, not end!
    intvlNumPackets = intvlNumBits = 0;
}

void ThruputMeter::finish()
{
    simtime_t duration = simTime() - startTime;

    recordScalar("duration", duration);
    recordScalar("total packets", numPackets);
    recordScalar("total bits", numBits);

    recordScalar("avg throughput (bit/s)", numBits / duration.dbl());
    recordScalar("avg packets/s", numPackets / duration.dbl());
}

} // namespace inet

