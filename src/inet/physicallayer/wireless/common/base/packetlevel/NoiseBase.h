//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_NOISEBASE_H
#define __INET_NOISEBASE_H

#include "inet/physicallayer/wireless/common/contract/packetlevel/INoise.h"

namespace inet {

namespace physicallayer {

class INET_API NoiseBase : public INoise
{
  protected:
    const simtime_t startTime;
    const simtime_t endTime;

  public:
    NoiseBase(simtime_t startTime, simtime_t endTime);

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;
    virtual const simtime_t getStartTime() const override { return startTime; }
    virtual const simtime_t getEndTime() const override { return endTime; }
};

} // namespace physicallayer

} // namespace inet

#endif

