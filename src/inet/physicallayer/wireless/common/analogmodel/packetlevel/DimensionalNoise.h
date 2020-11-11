//
// Copyright (C) 2013 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#ifndef __INET_DIMENSIONALNOISE_H
#define __INET_DIMENSIONALNOISE_H

#include "inet/common/math/Functions.h"
#include "inet/physicallayer/wireless/common/base/packetlevel/NarrowbandNoiseBase.h"

namespace inet {

namespace physicallayer {

using namespace inet::math;

class INET_API DimensionalNoise : public NarrowbandNoiseBase
{
  protected:
    const Ptr<const IFunction<WpHz, Domain<simsec, Hz>>> power;

  public:
    DimensionalNoise(simtime_t startTime, simtime_t endTime, Hz centerFrequency, Hz bandwidth, const Ptr<const IFunction<WpHz, Domain<simsec, Hz>>>& power);

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;
    virtual const Ptr<const IFunction<WpHz, Domain<simsec, Hz>>>& getPower() const { return power; }

    virtual W computeMinPower(simtime_t startTime, simtime_t endTime) const override;
    virtual W computeMaxPower(simtime_t startTime, simtime_t endTime) const override;
};

} // namespace physicallayer

} // namespace inet

#endif

