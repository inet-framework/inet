//
// Copyright (C) 2013 OpenSim Ltd.
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

#ifndef __INET_DIMENSIONALSIGNALANALOGMODEL_H
#define __INET_DIMENSIONALSIGNALANALOGMODEL_H

#include "inet/physicallayer/analogmodel/bitlevel/SignalAnalogModel.h"
#include "inet/common/mapping/MappingBase.h"
#include "inet/common/mapping/MappingUtils.h"

namespace inet {

namespace physicallayer {

class INET_API DimensionalSignalAnalogModel : public NarrowbandSignalAnalogModel, public IDimensionalSignal
{
  protected:
    const ConstMapping *power;

  public:
    DimensionalSignalAnalogModel(const simtime_t duration, Hz carrierFrequency, Hz bandwidth, const ConstMapping *power);
    virtual ~DimensionalSignalAnalogModel() { delete power; }

    virtual std::ostream& printToStream(std::ostream& stream, int level) const override;

    virtual const ConstMapping *getPower() const override { return power; }
    virtual W computeMinPower(simtime_t startTime, simtime_t endTime) const override;
};

class INET_API DimensionalTransmissionSignalAnalogModel : public DimensionalSignalAnalogModel, public virtual ITransmissionAnalogModel
{
  public:
    DimensionalTransmissionSignalAnalogModel(const simtime_t duration, Hz carrierFrequency, Hz bandwidth, const ConstMapping *power);
};

class INET_API DimensionalReceptionSignalAnalogModel : public DimensionalSignalAnalogModel, public virtual IReceptionAnalogModel
{
  public:
    DimensionalReceptionSignalAnalogModel(const simtime_t duration, Hz carrierFrequency, Hz bandwidth, const ConstMapping *power);
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_DIMENSIONALSIGNALANALOGMODEL_H

