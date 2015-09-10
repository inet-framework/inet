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

#ifndef __INET_DIMENSIONALTRANSMITTERBASE_H
#define __INET_DIMENSIONALTRANSMITTERBASE_H

#include "inet/common/mapping/MappingBase.h"
#include "inet/physicallayer/base/packetlevel/PhysicalLayerDefs.h"

namespace inet {

namespace physicallayer {

class INET_API DimensionalTransmitterBase : public virtual IPrintableObject
{
  protected:
    class TimeGainEntry {
      public:
        char timeUnit;
        double time;
        double gain;

      public:
        TimeGainEntry(char timeUnit, double time, double gain) :
            timeUnit(timeUnit),
            time(time),
            gain(gain)
        {}
    };

    class FrequencyGainEntry {
      public:
        char frequencyUnit;
        double frequency;
        double gain;

      public:
        FrequencyGainEntry(char frequencyUnit, double frequency, double gain) :
            frequencyUnit(frequencyUnit),
            frequency(frequency),
            gain(gain)
        {}
    };

  protected:
    DimensionSet dimensions;
    Mapping::InterpolationMethod interpolationMode;
    std::vector<TimeGainEntry> timeGains;
    std::vector<FrequencyGainEntry> frequencyGains;

  protected:
    virtual void initialize(int stage);

    virtual ConstMapping *createPowerMapping(const simtime_t startTime, const simtime_t endTime, Hz carrierFrequency, Hz bandwidth, W power) const;

  public:
    DimensionalTransmitterBase();

    virtual std::ostream& printToStream(std::ostream& stream, int level) const override;
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_DIMENSIONALTRANSMITTERBASE_H

