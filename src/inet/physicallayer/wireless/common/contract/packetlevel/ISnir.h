//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ISNIR_H
#define __INET_ISNIR_H

#include "inet/physicallayer/wireless/common/contract/packetlevel/INoise.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IReception.h"

namespace inet {

namespace physicallayer {

class INET_API ISnir : public virtual IPrintableObject
{
  public:
    virtual const IReception *getReception() const = 0;

    virtual const INoise *getNoise() const = 0;

    virtual double getMin() const = 0;
    virtual double getMax() const = 0;
    virtual double getMean() const = 0;

    /**
     * Returns the minimum, maximum, or mean SNIR in the provided time interval,
     * which lies inside the reception. A model of a part of the signal, for
     * example a chunk of the data, uses the SNIR of the time of that part.
     * If startTime is equal to endTime, the functions return the SNIR at
     * that time.
     */
    virtual double getMin(simtime_t startTime, simtime_t endTime) const = 0;
    virtual double getMax(simtime_t startTime, simtime_t endTime) const = 0;
    virtual double getMean(simtime_t startTime, simtime_t endTime) const = 0;
};

} // namespace physicallayer

} // namespace inet

#endif

