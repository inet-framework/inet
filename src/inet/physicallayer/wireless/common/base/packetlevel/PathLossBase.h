//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PATHLOSSBASE_H
#define __INET_PATHLOSSBASE_H

#include "inet/physicallayer/wireless/common/contract/packetlevel/IPathLoss.h"

namespace inet {

namespace physicallayer {

class INET_API PathLossBase : public cModule, public IPathLoss
{
  public:
    virtual double computePathLoss(mps propagationSpeed, Hz frequency, m distance) const override = 0;
    virtual double computePathLoss(const ITransmission *transmission, const IArrival *arrival) const override;
};

} // namespace physicallayer

} // namespace inet

#endif

