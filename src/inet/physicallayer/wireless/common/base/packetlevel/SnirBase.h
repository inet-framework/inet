//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_SNIRBASE_H
#define __INET_SNIRBASE_H

#include "inet/physicallayer/wireless/common/contract/packetlevel/INoise.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IReception.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/ISnir.h"

namespace inet {

namespace physicallayer {

class INET_API SnirBase : public virtual ISnir
{
  protected:
    const IReception *reception;
    const INoise *noise;

  protected:
    /**
     * Returns true if the interval is the whole reception. The subclasses
     * keep the values of the whole reception, so an interval getter can
     * return them.
     */
    bool isWholeReception(simtime_t startTime, simtime_t endTime) const { return startTime == reception->getStartTime() && endTime == reception->getEndTime(); }

  public:
    SnirBase(const IReception *reception, const INoise *noise);

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;
    virtual const IReception *getReception() const override { return reception; }
    virtual const INoise *getNoise() const override { return noise; }
};

} // namespace physicallayer

} // namespace inet

#endif

