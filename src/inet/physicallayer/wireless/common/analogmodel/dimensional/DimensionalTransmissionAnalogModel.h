//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_DIMENSIONALTRANSMISSIONANALOGMODEL_H
#define __INET_DIMENSIONALTRANSMISSIONANALOGMODEL_H

#include "inet/physicallayer/wireless/common/analogmodel/dimensional/DimensionalSignalAnalogModel.h"

namespace inet {

namespace physicallayer {

class INET_API DimensionalTransmissionAnalogModel : public DimensionalSignalAnalogModel, public virtual ITransmissionAnalogModel
{
  public:
    DimensionalTransmissionAnalogModel(const simtime_t preambleDuration, const simtime_t headerDuration, const simtime_t dataDuration, Hz centerFrequency, Hz bandwidth, const Ptr<const IFunction<WpHz, Domain<simsec, Hz>>>& power);
};

} // namespace physicallayer

} // namespace inet

#endif

