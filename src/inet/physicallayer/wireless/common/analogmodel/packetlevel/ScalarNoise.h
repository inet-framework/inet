//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_SCALARNOISE_H
#define __INET_SCALARNOISE_H

#include "inet/physicallayer/wireless/common/base/packetlevel/NarrowbandNoiseBase.h"

namespace inet {

namespace physicallayer {

class INET_API ScalarNoise : public NarrowbandNoiseBase
{
  protected:
    const std::map<simtime_t, W> *powerChanges;

  public:
    ScalarNoise(simtime_t startTime, simtime_t endTime, Hz centerFrequency, Hz bandwidth, const std::map<simtime_t, W> *powerChanges);
    virtual ~ScalarNoise() { delete powerChanges; }

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;
    virtual const std::map<simtime_t, W> *getPowerChanges() const { return powerChanges; }

    virtual W computeMinPower(simtime_t startTime, simtime_t endTime) const override;
    virtual W computeMaxPower(simtime_t startTime, simtime_t endTime) const override;
};

} // namespace physicallayer

} // namespace inet

#endif

