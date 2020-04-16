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

#ifndef __INET_TWORATETHREECOLORMETER_H
#define __INET_TWORATETHREECOLORMETER_H

#include "inet/common/INETDefs.h"
#include "inet/common/INETMath.h"
#include "inet/networklayer/diffserv/PacketMeterBase.h"

namespace inet {

/**
 * This class can be used as a meter in an ITrafficConditioner.
 * It marks the packets based on two rates, Peak Information Rate (PIR)
 * and Committed Information Rate (CIR), and their associated burst sizes
 * to be either green, yellow or red.
 *
 * See RFC 2698.
 */
class INET_API TwoRateThreeColorMeter : public PacketMeterBase
{
  protected:
    double PIR = NaN;    // Peak Information Rate (bits/sec)
    long PBS = 0;    // Peak Burst Size (bits)
    double CIR = NaN;    // Commited Information Rate (bits/sec)
    long CBS = 0;    // Committed Burst Size (bits)
    bool colorAwareMode = false;

    long Tp = 0;    // token bucket for peak burst
    long Tc = 0;    // token bucket for committed burst
    simtime_t lastUpdateTime;

    int numRcvd = 0;
    int numYellow = 0;
    int numRed = 0;

  public:
    TwoRateThreeColorMeter() {}

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void pushPacket(Packet *packet, cGate *gate) override;
    virtual void refreshDisplay() const override;

    virtual int meterPacket(Packet *packet) override;
};

} // namespace inet

#endif // ifndef __INET_TWORATETHREECOLORMETER_H

