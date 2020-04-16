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

#ifndef __INET_SCALARSIGNALANALOGMODEL_H
#define __INET_SCALARSIGNALANALOGMODEL_H

#include "inet/physicallayer/analogmodel/bitlevel/SignalAnalogModel.h"

namespace inet {

namespace physicallayer {

class INET_API ScalarSignalAnalogModel : public NarrowbandSignalAnalogModel, public virtual IScalarSignal
{
  protected:
    const W power;

  public:
    ScalarSignalAnalogModel(const simtime_t duration, Hz centerFrequency, Hz bandwidth, W power);

    virtual std::ostream& printToStream(std::ostream& stream, int level) const override;

    virtual W getPower() const override { return power; }
    virtual W computeMinPower(simtime_t startTime, simtime_t endTime) const override { return power; }
};

class INET_API ScalarTransmissionSignalAnalogModel : public ScalarSignalAnalogModel, public virtual ITransmissionAnalogModel
{
  public:
    ScalarTransmissionSignalAnalogModel(const simtime_t duration, Hz centerFrequency, Hz bandwidth, W power);
};

class INET_API ScalarReceptionSignalAnalogModel : public ScalarSignalAnalogModel, public virtual IReceptionAnalogModel
{
  public:
    ScalarReceptionSignalAnalogModel(const simtime_t duration, Hz centerFrequency, Hz bandwidth, W power);
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_SCALARSIGNALANALOGMODEL_H

