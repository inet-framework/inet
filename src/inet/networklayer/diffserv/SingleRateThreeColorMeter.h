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

#include "inet/common/INETDefs.h"
#include "inet/common/INETMath.h"

namespace inet {

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
    double CIR = NaN;    // Commited Information Rate (bits/sec)
    long CBS = 0;    // Committed Burst Size (bits)
    long EBS = 0;    // Excess Burst Size (bits)
    bool colorAwareMode = false;

    long Tc = 0;    // token bucket for committed burst
    long Te = 0;    // token bucket for excess burst
    simtime_t lastUpdateTime;

    int numRcvd = 0;
    int numYellow = 0;
    int numRed = 0;

  public:
    SingleRateThreeColorMeter() {}

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }

    virtual void initialize(int stage) override;

    virtual void handleMessage(cMessage *msg) override;

    virtual int meterPacket(cPacket *packet);
};

} // namespace inet

#endif // ifndef __INET_SINGLERATETHREECOLORMETER_H

