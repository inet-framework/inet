//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_FLATTRANSMISSIONBASE_H
#define __INET_FLATTRANSMISSIONBASE_H

#include "inet/physicallayer/wireless/common/base/packetlevel/NarrowbandTransmissionBase.h"

namespace inet {

namespace physicallayer {

class INET_API FlatTransmissionBase : public NarrowbandTransmissionBase
{
  protected:
    const b headerLength;
    const b dataLength;
    const bps bitrate;
    const double codeRate;

  public:
    FlatTransmissionBase(const IRadio *transmitter, const Packet *packet, const simtime_t startTime, const simtime_t endTime, const simtime_t preambleDuration, const simtime_t headerDuration, const simtime_t dataDuration, const Coord startPosition, const Coord endPosition, const Quaternion startOrientation, const Quaternion endOrientation, b headerLength, b dataLength, bps bitrate, double codeRate, const IModulation *modulation, const simtime_t symbolTime, Hz centerFrequency, Hz bandwidth);

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;

    virtual b getHeaderLength() const { return headerLength; }
    virtual b getDataLength() const { return dataLength; }
    virtual bps getBitrate() const { return bitrate; }
    virtual double getCodeRate() const { return codeRate; }
};

} // namespace physicallayer

} // namespace inet

#endif

