//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_SCALARTRANSMISSION_H
#define __INET_SCALARTRANSMISSION_H

#include "inet/physicallayer/wireless/common/base/packetlevel/FlatTransmissionBase.h"

namespace inet {

namespace physicallayer {

class INET_API ScalarTransmission : public FlatTransmissionBase, public virtual IScalarSignal
{
  protected:
    const W power;

  public:
    ScalarTransmission(const IRadio *transmitter, const Packet *packet, const simtime_t startTime, const simtime_t endTime, const simtime_t preambleDuration, const simtime_t headerDuration, const simtime_t dataDuration, const Coord startPosition, const Coord endPosition, const Quaternion startOrientation, const Quaternion endOrientation, b headerLength, b dataLength, const IModulation *modulation, const simtime_t symbolTime, Hz centerFrequency, Hz bandwidth, bps bitrate, double codeRate, W power);

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;

    virtual W getPower() const override { return power; }
    virtual W computeMinPower(const simtime_t startTime, const simtime_t endTime) const override { return power; }
};

} // namespace physicallayer

} // namespace inet

#endif

