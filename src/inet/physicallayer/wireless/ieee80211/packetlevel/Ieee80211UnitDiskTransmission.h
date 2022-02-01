//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IEEE80211UNITDISKTRANSMISSION_H
#define __INET_IEEE80211UNITDISKTRANSMISSION_H

#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211TransmissionBase.h"
#include "inet/physicallayer/wireless/unitdisk/UnitDiskTransmission.h"

namespace inet {

namespace physicallayer {

// TODO IdealTransmissionBase
class INET_API Ieee80211UnitDiskTransmission : public UnitDiskTransmission, public Ieee80211TransmissionBase
{
  public:
    Ieee80211UnitDiskTransmission(const IRadio *transmitter, const Packet *packet, const simtime_t startTime, const simtime_t endTime, const simtime_t preambleDuration, const simtime_t headerDuration, const simtime_t dataDuration, const Coord& startPosition, const Coord& endPosition, const Quaternion& startOrientation, const Quaternion& endOrientation, m communicationRange, m interferenceRange, m detectionRange, const IIeee80211Mode *mode, const Ieee80211Channel *channel);

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;
};

} // namespace physicallayer

} // namespace inet

#endif

