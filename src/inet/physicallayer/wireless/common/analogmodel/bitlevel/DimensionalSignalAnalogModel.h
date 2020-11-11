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

#ifndef __INET_DIMENSIONALSIGNALANALOGMODEL_H
#define __INET_DIMENSIONALSIGNALANALOGMODEL_H

#include "inet/common/math/Functions.h"
#include "inet/physicallayer/wireless/common/analogmodel/bitlevel/SignalAnalogModel.h"

namespace inet {

namespace physicallayer {

using namespace inet::math;

class INET_API DimensionalSignalAnalogModel : public NarrowbandSignalAnalogModel, public IDimensionalSignal
{
  protected:
    const Ptr<const IFunction<WpHz, Domain<simsec, Hz>>> power;

  public:
    DimensionalSignalAnalogModel(const simtime_t duration, Hz centerFrequency, Hz bandwidth, const Ptr<const IFunction<WpHz, Domain<simsec, Hz>>>& power);

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;

    virtual const Ptr<const IFunction<WpHz, Domain<simsec, Hz>>>& getPower() const override { return power; }
    virtual W computeMinPower(simtime_t startTime, simtime_t endTime) const override;
};

class INET_API DimensionalTransmissionSignalAnalogModel : public DimensionalSignalAnalogModel, public virtual ITransmissionAnalogModel
{
  public:
    DimensionalTransmissionSignalAnalogModel(const simtime_t duration, Hz centerFrequency, Hz bandwidth, const Ptr<const IFunction<WpHz, Domain<simsec, Hz>>>& power);
};

class INET_API DimensionalReceptionSignalAnalogModel : public DimensionalSignalAnalogModel, public virtual IReceptionAnalogModel
{
  public:
    DimensionalReceptionSignalAnalogModel(const simtime_t duration, Hz centerFrequency, Hz bandwidth, const Ptr<const IFunction<WpHz, Domain<simsec, Hz>>>& power);
};

} // namespace physicallayer

} // namespace inet

#endif

