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
  protected:
    const m communicationRange;
    const m interferenceRange;
    const m detectionRange;

  public:
    UnitDiskTransmission(const IRadio *transmitter, const Packet *macFrame, const simtime_t startTime, const simtime_t endTime, const simtime_t preambleDuration, const simtime_t headerDuration, const simtime_t dataDuration, const Coord& startPosition, const Coord& endPosition, const Quaternion& startOrientation, const Quaternion& endOrientation, m communicationRange, m interferenceRange, m detectionRange);

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;

    virtual m getCommunicationRange() const { return communicationRange; }
    virtual m getInterferenceRange() const { return interferenceRange; }
    virtual m getDetectionRange() const { return detectionRange; }
};

} // namespace physicallayer

} // namespace inet

#endif

