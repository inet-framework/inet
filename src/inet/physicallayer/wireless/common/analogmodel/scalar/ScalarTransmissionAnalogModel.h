//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_SCALARTRANSMISSIONANALOGMODEL_H
#define __INET_SCALARTRANSMISSIONANALOGMODEL_H

#include "inet/physicallayer/wireless/common/analogmodel/scalar/ScalarSignalAnalogModel.h"

namespace inet {

namespace physicallayer {

class INET_API ScalarTransmissionAnalogModel : public ScalarSignalAnalogModel, public virtual ITransmissionAnalogModel
{
  public:
    ScalarTransmissionAnalogModel(const simtime_t preambleDuration, simtime_t headerDuration, simtime_t dataDuration, Hz centerFrequency, Hz bandwidth, W power);
};

} // namespace physicallayer

} // namespace inet

#endif

