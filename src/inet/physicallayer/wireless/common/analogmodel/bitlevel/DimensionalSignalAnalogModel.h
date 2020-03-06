//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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
    DimensionalSignalAnalogModel(const simtime_t preambleDuration, const simtime_t headerDuration, const simtime_t dataDuration, Hz centerFrequency, Hz bandwidth, const Ptr<const IFunction<WpHz, Domain<simsec, Hz>>>& power);

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;

    virtual const Ptr<const IFunction<WpHz, Domain<simsec, Hz>>>& getPower() const override { return power; }
    virtual W computeMinPower(simtime_t startTime, simtime_t endTime) const override;
};

class INET_API DimensionalTransmissionSignalAnalogModel : public DimensionalSignalAnalogModel, public virtual ITransmissionAnalogModel
{
  public:
    DimensionalTransmissionSignalAnalogModel(const simtime_t preambleDuration, const simtime_t headerDuration, const simtime_t dataDuration, Hz centerFrequency, Hz bandwidth, const Ptr<const IFunction<WpHz, Domain<simsec, Hz>>>& power);
};

class INET_API DimensionalReceptionSignalAnalogModel : public DimensionalSignalAnalogModel, public virtual IReceptionAnalogModel
{
  public:
    DimensionalReceptionSignalAnalogModel(const simtime_t preambleDuration, const simtime_t headerDuration, const simtime_t dataDuration, Hz centerFrequency, Hz bandwidth, const Ptr<const IFunction<WpHz, Domain<simsec, Hz>>>& power);
};

} // namespace physicallayer

} // namespace inet

#endif

