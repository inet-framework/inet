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


#ifndef __INET_THRUPUTMETER2_H
#define __INET_THRUPUTMETER2_H

#include <omnetpp.h>
#include "INETDefs.h"


/**
 * Measures and records network throughput
 */
class INET_API ThruputMeter2 : public cSimpleModule
{
  protected:
    // config
    simtime_t startTime;            // start time
    simtime_t measurementInterval;  // measurement interval

    // global statistics
    unsigned long numPackets;
    unsigned long numBits;

    // current measurement interval
    unsigned long intvlNumPackets;
    unsigned long intvlNumBits;

    // statistics
    cOutVector bitpersecVector;
    cOutVector pkpersecVector;

    // timer
    bool measurementStarted;
    cMessage *measurementTimer;

  public:
    ThruputMeter2();
    virtual ~ThruputMeter2();

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual void finish();
};

#endif
