//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IOBSTACLELOSS_H
#define __INET_IOBSTACLELOSS_H

#include "inet/common/IPrintableObject.h"
#include "inet/common/geometry/common/Coord.h"

namespace inet {

namespace physicallayer {

/**
 * This interface models obstacle loss that is the reduction in power density of
 * a radio signal as it propagates through physical objects present in space.
 */
class INET_API IObstacleLoss : public IPrintableObject
{
  public:
    /**
     * Returns the obstacle loss factor caused by physical objects present in
     * the environment as a function of frequency, transmission position, and
     * reception position. The value is in the range [0, 1] where 1 means no
     * loss at all and 0 means all power is lost.
     */
    virtual double computeObstacleLoss(Hz frequency, const Coord& transmissionPosition, const Coord& receptionPosition) const = 0;
};

} // namespace physicallayer

} // namespace inet

#endif

