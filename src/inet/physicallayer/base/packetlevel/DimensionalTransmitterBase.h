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

#include "inet/physicallayer/contract/packetlevel/IPrintableObject.h"

namespace inet {

namespace physicallayer {

class INET_API DimensionalTransmitterBase : public virtual IPrintableObject
{
  protected:
    template<typename T>
    class GainEntry {
      public:
        const math::IInterpolator<T, double> *interpolator;
        const char where;
        double length;
        T offset;
        double gain;

      public:
        GainEntry(const math::IInterpolator<T, double> *interpolator, const char where, double length, T offset, double gain) :
            interpolator(interpolator), where(where), length(length), offset(offset), gain(gain) { }
    };

  protected:
    const math::IInterpolator<simtime_t, double> *firstTimeInterpolator = nullptr;
    const math::IInterpolator<Hz, double> *firstFrequencyInterpolator = nullptr;
    std::vector<GainEntry<simtime_t>> timeGains;
    std::vector<GainEntry<Hz>> frequencyGains;

  protected:
    virtual void initialize(int stage);

    template<typename T>
    std::vector<GainEntry<T>> parseGains(const char *text) const;

    virtual void parseTimeGains(const char *text);
    virtual void parseFrequencyGains(const char *text);

    virtual Ptr<const math::IFunction<W, simtime_t, Hz>> createPowerFunction(const simtime_t startTime, const simtime_t endTime, Hz carrierFrequency, Hz bandwidth, W power) const;

  public:
    virtual std::ostream& printToStream(std::ostream& stream, int level) const override;
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_DIMENSIONALTRANSMITTERBASE_H

