//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_SCALARNOISE_H
#define __INET_SCALARNOISE_H

#include "inet/physicallayer/wireless/common/base/packetlevel/NarrowbandNoiseBase.h"

#include "inet/common/math/IFunction.h"

namespace inet {

namespace physicallayer {

class INET_API ScalarNoise : public NarrowbandNoiseBase
{
  public:
    // Each contribution has uniform spectral density within its own band.
    struct PowerComponent {
        Hz centerFrequency;
        Hz bandwidth;
        Ptr<const math::IFunction<W, math::Domain<simtime_t>>> powerFunction;
    };

  protected:
    Ptr<const math::IFunction<W, math::Domain<simtime_t>>> powerFunction;
    const std::vector<PowerComponent> powerComponents;

  public:
    // Without explicit components, powerFunction is uniform over the given band.
    ScalarNoise(simtime_t startTime, simtime_t endTime, Hz centerFrequency, Hz bandwidth, Ptr<const math::IFunction<W, math::Domain<simtime_t>>> powerFunction, const std::vector<PowerComponent>& powerComponents = {});

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;
    virtual Ptr<const math::IFunction<W, math::Domain<simtime_t>>> getPower() const { return powerFunction; }

    const std::vector<PowerComponent>& getPowerComponents() const { return powerComponents; }
    // Integrate each flat spectral contribution over the requested band before summing.
    virtual Ptr<const math::IFunction<W, math::Domain<simtime_t>>> getPower(Hz centerFrequency, Hz bandwidth) const;

    virtual W computeMinPower(simtime_t startTime, simtime_t endTime) const override;
    virtual W computeMaxPower(simtime_t startTime, simtime_t endTime) const override;
};

} // namespace physicallayer

} // namespace inet

#endif

