//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_CONSTANTTIMEPROPAGATION_H
#define __INET_CONSTANTTIMEPROPAGATION_H

#include "inet/physicallayer/wireless/common/base/packetlevel/PropagationBase.h"

namespace inet {

namespace physicallayer {

class INET_API ConstantTimePropagation : public PropagationBase
{
  protected:
    simtime_t propagationTime;

  protected:
    void initialize(int stage) override;

  public:
    ConstantTimePropagation();

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;

    virtual const IArrival *computeArrival(const ITransmission *transmission, IMobility *mobility) const override;
};

} // namespace physicallayer

} // namespace inet

#endif

