//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_INARROWBANDSIGNAL_H
#define __INET_INARROWBANDSIGNAL_H

#include "inet/common/Units.h"
#include "inet/physicallayer/wireless/common/contract/bitlevel/ISignalAnalogModel.h"

namespace inet {
namespace physicallayer {

using namespace units::values;

class INET_API INarrowbandSignalAnalogModel : public virtual ISignalAnalogModel
{
  public:
    virtual Hz getCenterFrequency() const = 0;
    virtual Hz getBandwidth() const = 0;

    virtual W computeMinPower(simtime_t startTime, simtime_t endTime) const = 0;
};

} // namespace physicallayer
} // namespace inet

#endif

