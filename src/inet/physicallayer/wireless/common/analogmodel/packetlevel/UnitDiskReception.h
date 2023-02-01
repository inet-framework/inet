//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_UNITDISKRECEPTION_H
#define __INET_UNITDISKRECEPTION_H

#include "inet/physicallayer/wireless/common/base/packetlevel/ReceptionBase.h"

namespace inet {

namespace physicallayer {

/**
 * This model characterizes receptions with a simplified reception power that
 * falls into one of the categories: receivable, interfering, detectable, and
 * undetectable.
 */
class INET_API UnitDiskReception : public ReceptionBase
{
  public:
    enum Power {
        POWER_UNDETECTABLE,
        POWER_DETECTABLE,
        POWER_INTERFERING,
        POWER_RECEIVABLE
    };

  protected:
    const Power power;

  public:
    UnitDiskReception(const IRadio *radio, const ITransmission *transmission, const simtime_t startTime, const simtime_t endTime, const Coord& startPosition, const Coord& endPosition, const Quaternion& startOrientation, const Quaternion& endOrientation, const Power power);

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;
    virtual Power getPower() const { return power; }
};

} // namespace physicallayer

} // namespace inet

#endif

