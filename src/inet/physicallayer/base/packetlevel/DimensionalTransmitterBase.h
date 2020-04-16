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

#include "inet/common/math/IFunction.h"
#include "inet/common/math/IInterpolator.h"
#include "inet/physicallayer/contract/packetlevel/IPrintableObject.h"

namespace inet {

namespace physicallayer {

using namespace inet::math;

class INET_API DimensionalTransmitterBase : public virtual IPrintableObject
{
  protected:
    template<typename T>
    class GainEntry {
      public:
        const IInterpolator<T, double> *interpolator;
        const char where;
        double length;
        T offset;
        double gain;

      public:
        GainEntry(const IInterpolator<T, double> *interpolator, const char where, double length, T offset, double gain) :
            interpolator(interpolator), where(where), length(length), offset(offset), gain(gain) { }
    };

  protected:
    const IInterpolator<simsec, double> *firstTimeInterpolator = nullptr;
    const IInterpolator<Hz, double> *firstFrequencyInterpolator = nullptr;
    std::vector<GainEntry<simsec>> timeGains;
    std::vector<GainEntry<Hz>> frequencyGains;
    const char *timeGainsNormalization = nullptr;
    const char *frequencyGainsNormalization = nullptr;

    int gainFunctionCacheLimit = -1;
    mutable std::map<std::tuple<simtime_t, Hz, Hz>, Ptr<const IFunction<double, Domain<simsec, Hz>>>> gainFunctionCache;

  protected:
    virtual void initialize(int stage);

    template<typename T>
    std::vector<GainEntry<T>> parseGains(const char *text) const;

    virtual void parseTimeGains(const char *text);
    virtual void parseFrequencyGains(const char *text);

    template<typename T>
    const Ptr<const IFunction<double, Domain<T>>> normalize(const Ptr<const IFunction<double, Domain<T>>>& function, const char *normalization) const;

    virtual Ptr<const IFunction<double, Domain<simsec, Hz>>> createGainFunction(const simtime_t startTime, const simtime_t endTime, Hz centerFrequency, Hz bandwidth) const;
    virtual Ptr<const IFunction<WpHz, Domain<simsec, Hz>>> createPowerFunction(const simtime_t startTime, const simtime_t endTime, Hz centerFrequency, Hz bandwidth, W power) const;

  public:
    virtual std::ostream& printToStream(std::ostream& stream, int level) const override;
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_DIMENSIONALTRANSMITTERBASE_H

