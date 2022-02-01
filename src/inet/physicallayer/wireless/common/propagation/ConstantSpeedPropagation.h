//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_CONSTANTSPEEDPROPAGATION_H
#define __INET_CONSTANTSPEEDPROPAGATION_H

#include "inet/physicallayer/wireless/common/base/packetlevel/PropagationBase.h"

namespace inet {

namespace physicallayer {

class INET_API ConstantSpeedPropagation : public PropagationBase
{
  protected:
    bool ignoreMovementDuringTransmission;
    bool ignoreMovementDuringPropagation;
    bool ignoreMovementDuringReception;

  protected:
    virtual void initialize(int stage) override;
    virtual const Coord computeArrivalPosition(const simtime_t startTime, const Coord& startPosition, IMobility *mobility) const;

  public:
    ConstantSpeedPropagation();

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;
    virtual const IArrival *computeArrival(const ITransmission *transmission, IMobility *mobility) const override;
};

} // namespace physicallayer

} // namespace inet

#endif

