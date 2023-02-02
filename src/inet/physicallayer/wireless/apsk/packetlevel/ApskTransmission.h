//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_APSKTRANSMISSION_H
#define __INET_APSKTRANSMISSION_H

#include "inet/physicallayer/wireless/common/base/packetlevel/TransmissionBase.h"

namespace inet {

namespace physicallayer {

class INET_API ApskTransmission : public TransmissionBase
{
  protected:
    const b headerLength = b(-1);
    const b dataLength = b(-1);
    const IModulation *modulation = nullptr;
    const Hz bandwidth = Hz(NaN);
    const simtime_t symbolTime;
    const bps bitrate = bps(NaN);
    const double codeRate = NaN;

  public:
    ApskTransmission(const IRadio *transmitter, const Packet *packet, const simtime_t startTime, const simtime_t endTime, const simtime_t preambleDuration, const simtime_t headerDuration, const simtime_t dataDuration, const Coord startPosition, const Coord endPosition, const Quaternion startOrientation, const Quaternion endOrientation, b headerLength, b dataLength, const IModulation *modulation, Hz bandwidth, const simtime_t symbolTime, bps bitrate, double codeRate);

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;

    virtual bps getBitrate() const { return bitrate; }
    virtual double getCodeRate() const { return codeRate; }
    virtual b getHeaderLength() const { return headerLength; }
    virtual const IModulation *getModulation() const { return modulation; }
    virtual const Hz getBandwidth() const { return bandwidth; }
    virtual b getDataLength() const { return dataLength; }
    virtual const simtime_t& getSymbolTime() const { return symbolTime; }
};

} // namespace physicallayer

} // namespace inet

#endif

