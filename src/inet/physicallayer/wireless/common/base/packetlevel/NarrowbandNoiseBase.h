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

#ifndef __INET_NARROWBANDNOISEBASE_H
#define __INET_NARROWBANDNOISEBASE_H

#include "inet/common/Units.h"
#include "inet/physicallayer/wireless/common/base/packetlevel/NoiseBase.h"

namespace inet {

namespace physicallayer {

using namespace inet::units::values;

class INET_API NarrowbandNoiseBase : public NoiseBase
{
  protected:
    const Hz centerFrequency;
    const Hz bandwidth;

  public:
    NarrowbandNoiseBase(simtime_t startTime, simtime_t endTime, Hz centerFrequency, Hz bandwidth);

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;

    virtual Hz getCenterFrequency() const { return centerFrequency; }
    virtual Hz getBandwidth() const { return bandwidth; }

    virtual W computeMinPower(simtime_t startTime, simtime_t endTime) const = 0;
    virtual W computeMaxPower(simtime_t startTime, simtime_t endTime) const = 0;
};

} // namespace physicallayer

} // namespace inet

#endif

