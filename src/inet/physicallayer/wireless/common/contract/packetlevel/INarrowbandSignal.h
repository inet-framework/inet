//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_INARROWBANDSIGNAL_H
#define __INET_INARROWBANDSIGNAL_H

#include "inet/common/IPrintableObject.h"
#include "inet/common/geometry/common/Coord.h"
#include "inet/common/math/Functions.h"

namespace inet {
namespace physicallayer {

class INET_API INarrowbandSignal : public virtual IRadioSignal
{
  public:
    virtual Hz getCenterFrequency() const = 0;
    virtual Hz getBandwidth() const = 0;

    virtual W computeMinPower(simtime_t startTime, simtime_t endTime) const = 0;
};

} // namespace physicallayer
} // namespace inet

#endif

