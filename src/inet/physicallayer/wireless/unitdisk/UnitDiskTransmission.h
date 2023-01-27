//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_UNITDISKTRANSMISSION_H
#define __INET_UNITDISKTRANSMISSION_H

#include "inet/physicallayer/wireless/common/base/packetlevel/TransmissionBase.h"

namespace inet {

namespace physicallayer {

/**
 * This model characterizes transmissions with the communication range,
 * interference range, and detection range.
 */
class INET_API UnitDiskTransmission : public TransmissionBase
{
  public:
    UnitDiskTransmission(const IRadio *transmitter, const Packet *macFrame, const simtime_t startTime, const simtime_t endTime, const simtime_t preambleDuration, const simtime_t headerDuration, const simtime_t dataDuration, const Coord& startPosition, const Coord& endPosition, const Quaternion& startOrientation, const Quaternion& endOrientation);
};

} // namespace physicallayer

} // namespace inet

#endif

