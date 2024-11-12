//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_PISERVOCLOCK_H
#define __INET_PISERVOCLOCK_H

#include "ServoClockBase.h"

namespace inet {

class INET_API PiServoClock : public ServoClockBase
{
  protected:
    static simsignal_t driftSignal;
    static simsignal_t kpSignal;

    int phase = 0;
    clocktime_t offset[2];
    clocktime_t local[2];

    cPar *offsetThreshold;

    double kp;   // proportional gain
    double ki; // integral gain

    ppm kpTerm = ppm(0);
    ppm kiTerm = ppm(0);

    ppm kpTermMax = ppm(100);
    ppm kpTermMin = ppm(-100);
    ppm kiTermMax = ppm(100);
    ppm kiTermMin = ppm(-100);

    ppm drift = ppm(0);

  protected:
    virtual void initialize(int stage) override;

  public:
    void adjustClockTo(clocktime_t newClockTime) override;
};

} // namespace inet

#endif
