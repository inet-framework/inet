//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_DIMENSIONALRECEPTIONANALOGMODEL_H
#define __INET_DIMENSIONALRECEPTIONANALOGMODEL_H

#include "inet/common/Units.h"
#include "inet/common/math/Functions.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/INewReceptionAnalogModel.h"

namespace inet {

namespace physicallayer {

using namespace inet::math;
using namespace inet::units::values;

class INET_API DimensionalReceptionAnalogModel : public INewReceptionAnalogModel
{
  protected:
    // TODO delete the frequencies?
    const Hz centerFrequency = Hz(NaN);
    const Hz bandwidth = Hz(NaN);
    const Ptr<const IFunction<WpHz, Domain<simsec, Hz>>> powerFunction;

  public:
    DimensionalReceptionAnalogModel(const Ptr<const IFunction<WpHz, Domain<simsec, Hz>>> powerFunction) : powerFunction(powerFunction) {}

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;

    // TODO delete the frequencies?
    virtual const Hz getBandwidth() const { return bandwidth; }
    virtual const Hz getCenterFrequency() const { return centerFrequency; }
    virtual const Ptr<const IFunction<WpHz, Domain<simsec, Hz>>> getPower() const { return powerFunction; }
};

} // namespace physicallayer

} // namespace inet

#endif

