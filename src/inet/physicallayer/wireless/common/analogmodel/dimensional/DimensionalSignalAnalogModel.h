//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_DIMENSIONALSIGNALANALOGMODEL_H
#define __INET_DIMENSIONALSIGNALANALOGMODEL_H

#include "inet/common/math/IFunction.h"
#include "inet/physicallayer/wireless/common/analogmodel/common/NarrowbandSignalAnalogModel.h"
#include "../../contract/packetlevel/IDimensionalSignalAnalogModel.h"

namespace inet {

namespace physicallayer {

using namespace inet::math;

class INET_API DimensionalSignalAnalogModel : public NarrowbandSignalAnalogModel, public IDimensionalSignalAnalogModel
{
  protected:
    const Ptr<const IFunction<WpHz, Domain<simsec, Hz>>> power;

  public:
    DimensionalSignalAnalogModel(const simtime_t preambleDuration, const simtime_t headerDuration, const simtime_t dataDuration, Hz centerFrequency, Hz bandwidth, const Ptr<const IFunction<WpHz, Domain<simsec, Hz>>>& power);

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;

    virtual const Ptr<const IFunction<WpHz, Domain<simsec, Hz>>>& getPower() const override { return power; }
    virtual W computeMinPower(simtime_t startTime, simtime_t endTime) const override;
};

} // namespace physicallayer

} // namespace inet

#endif

