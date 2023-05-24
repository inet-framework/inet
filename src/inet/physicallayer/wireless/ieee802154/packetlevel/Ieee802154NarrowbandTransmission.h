//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IEEE802154TRANSMISSION_H
#define __INET_IEEE802154TRANSMISSION_H

#include "inet/physicallayer/wireless/common/base/packetlevel/FlatTransmissionBase.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadio.h"

namespace inet {

namespace physicallayer {

class INET_API Ieee802154NarrowbandTransmission : public FlatTransmissionBase
{
  public:
    Ieee802154NarrowbandTransmission(const IRadio *transmitter, const Packet *packet, const simtime_t startTime, const simtime_t endTime, const simtime_t preambleDuration, const simtime_t headerDuration, const simtime_t dataDuration, const Coord startPosition, const Coord endPosition, const Quaternion startOrientation, const Quaternion endOrientation, b headerLength, b dataLength, const IModulation *modulation, const simtime_t symbolTime, Hz centerFrequency, Hz bandwidth, bps bitrate, double codeRate, W power);

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;

    // TODO KLUDGE replace this with analog model
    virtual W computeMinPower(simtime_t startTime, simtime_t endTime) const override { throw cRuntimeError("KLUDGE"); }
};

} // namespace physicallayer

} // namespace inet

#endif

