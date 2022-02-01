//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_FLATRECEPTIONBASE_H
#define __INET_FLATRECEPTIONBASE_H

#include "inet/physicallayer/wireless/common/base/packetlevel/NarrowbandReceptionBase.h"

namespace inet {

namespace physicallayer {

class INET_API FlatReceptionBase : public NarrowbandReceptionBase
{
  public:
    FlatReceptionBase(const IRadio *receiver, const ITransmission *transmission, const simtime_t startTime, const simtime_t endTime, const Coord& startPosition, const Coord& endPosition, const Quaternion& startOrientation, const Quaternion& endOrientation, Hz centerFrequency, Hz bandwidth);

    virtual W computeMinPower(simtime_t startTime, simtime_t endTime) const = 0;
};

} // namespace physicallayer

} // namespace inet

#endif

