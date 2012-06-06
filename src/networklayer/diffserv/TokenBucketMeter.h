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


#ifndef __INET_TOKENBUCKETMETER_H
#define __INET_TOKENBUCKETMETER_H

#include "INETDefs.h"

/**
 * Simple token bucket meter.
 */
class INET_API TokenBucketMeter : public cSimpleModule
{
  protected:
    double CIR; // Commited Information Rate (bits/sec)
    long CBS;    // Committed Burst Size (in bits)
    bool colorAwareMode;

    long Tc; // token bucket for committed burst
    simtime_t lastUpdateTime;

    int numRcvd;
    int numRed;

  public:
    TokenBucketMeter() {}

  protected:

    virtual int numInitStages() const { return 3; }

    virtual void initialize(int stage);

    virtual void handleMessage(cMessage *msg);

    virtual int meterPacket(cPacket *packet);
};

#endif
