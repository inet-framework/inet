//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_DIMENSIONALTRANSMITTERANALOGMODEL_H
#define __INET_DIMENSIONALTRANSMITTERANALOGMODEL_H

#include "inet/physicallayer/wireless/common/analogmodel/dimensional/DimensionalSignalAnalogModel.h"
#include "inet/physicallayer/wireless/common/base/packetlevel/TransmitterAnalogModelBase.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/ITransmitterAnalogModel.h"
#include "inet/physicallayer/wireless/common/contract/bitlevel/ISignalAnalogModel.h"

namespace inet {

namespace physicallayer {

class INET_API DimensionalTransmitterAnalogModel : public TransmitterAnalogModelBase, public ITransmitterAnalogModel
{
  protected:
    template<typename T>
    class INET_API GainEntry {
      public:
        const IInterpolator<T, double> *interpolator;
        const char where;
        double length;
        T offset;
        double gain;

      public:
        GainEntry(const IInterpolator<T, double> *interpolator, const char where, double length, T offset, double gain) :
            interpolator(interpolator), where(where), length(length), offset(offset), gain(gain) {}
    };

  protected:
    Hz defaultCenterFrequency = Hz(NaN);
    Hz defaultBandwidth = Hz(NaN);
    W defaultPower = W(NaN);

    const IInterpolator<simsec, double> *firstTimeInterpolator = nullptr;
    const IInterpolator<Hz, double> *firstFrequencyInterpolator = nullptr;
    std::vector<GainEntry<simsec>> timeGains;
    std::vector<GainEntry<Hz>> frequencyGains;
    const char *timeGainsNormalization = nullptr;
    const char *frequencyGainsNormalization = nullptr;

    int gainFunctionCacheLimit = -1;
    mutable std::map<std::tuple<simtime_t, Hz, Hz>, Ptr<const IFunction<double, Domain<simsec, Hz>>>> gainFunctionCache;

  protected:
    virtual void initialize(int stage) override;

    virtual Hz computeCenterFrequency(Hz centerFrequency) const {
        return std::isnan(centerFrequency.get()) ? defaultCenterFrequency : centerFrequency;
    }

    virtual Hz computeBandwidth(Hz bandwidth) const {
        return std::isnan(bandwidth.get()) ? defaultBandwidth : bandwidth;
    }

    virtual W computePower(W power) const {
        return std::isnan(power.get()) ? defaultPower : power;
    }

    template<typename T>
    std::vector<GainEntry<T>> parseGains(const char *text) const;

    virtual void parseTimeGains(const char *text);
    virtual void parseFrequencyGains(const char *text);

    template<typename T>
    const Ptr<const IFunction<double, Domain<T>>> normalize(const Ptr<const IFunction<double, Domain<T>>>& function, const char *normalization) const;

    virtual Ptr<const IFunction<double, Domain<simsec, Hz>>> createGainFunction(const simtime_t startTime, const simtime_t endTime, Hz centerFrequency, Hz bandwidth) const;
    virtual Ptr<const IFunction<WpHz, Domain<simsec, Hz>>> createPowerFunction(const simtime_t startTime, const simtime_t endTime, Hz centerFrequency, Hz bandwidth, W power) const;

  public:
    virtual ITransmissionAnalogModel* createAnalogModel(const Packet *packet, simtime_t duration, Hz centerFrequency, Hz bandwidth, W power) const override;
};

} // namespace physicallayer

} // namespace inet

#endif

