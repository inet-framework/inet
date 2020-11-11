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

