//
// Copyright (C) 2005 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_THRUPUTMETER_H
#define __INET_THRUPUTMETER_H

#include "inet/common/INETDefs.h"

namespace inet {

/**
 * Measures and records network thruput
 */
// FIXME problem: if traffic suddenly stops, it'll show the last reading forever;
// (output vector will be correct though); would need a timer to handle this situation
class INET_API ThruputMeter : public cSimpleModule
{
  protected:
    // config
    simtime_t startTime; // start time
    unsigned int batchSize; // number of packets in a batch
    simtime_t maxInterval; // max length of measurement interval (measurement ends
    // if either batchSize or maxInterval is reached, whichever
    // is reached first)

    // global statistics
    unsigned long numPackets;
    unsigned long numBits;

    // current measurement interval
    simtime_t intvlStartTime;
    simtime_t intvlLastPkTime;
    unsigned long intvlNumPackets;
    unsigned long intvlNumBits;

    // statistics
    cOutVector bitpersecVector;
    cOutVector pkpersecVector;

  protected:
    virtual void updateStats(simtime_t now, unsigned long bits);
    virtual void beginNewInterval(simtime_t now);

  protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;
};

} // namespace inet

#endif

