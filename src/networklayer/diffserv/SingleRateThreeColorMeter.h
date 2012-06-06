//
// Copyright (C) 2012 Opensim Ltd.
// Author: Tamas Borbely
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


#ifndef __INET_SINGLERATETHREECOLORMETER_H
#define __INET_SINGLERATETHREECOLORMETER_H

#include "INETDefs.h"

/**
 * This class can be used as a meter in an ITrafficConditioner.
 * It marks the packets according to three parameters,
 * Committed Information Rate (CIR), Committed Burst Size (CBS),
 * and Excess Burst Size (EBS), to be either green, yellow or red.
 *
 * See RFC 2697.
 */
class INET_API SingleRateThreeColorMeter : public cSimpleModule
{
  protected:
    double CIR; // Commited Information Rate (bits/sec)
    long CBS; // Committed Burst Size (bits)
    long EBS; // Excess Burst Size (bits)
    bool colorAwareMode;

    long Tc; // token bucket for committed burst
    long Te; // token bucket for excess burst
    simtime_t lastUpdateTime;

    int numRcvd;
    int numYellow;
    int numRed;

  public:
    SingleRateThreeColorMeter() {}

  protected:

    virtual int numInitStages() const { return 3; }

    virtual void initialize(int stage);

    virtual void handleMessage(cMessage *msg);

    virtual int meterPacket(cPacket *packet);
};

#endif
