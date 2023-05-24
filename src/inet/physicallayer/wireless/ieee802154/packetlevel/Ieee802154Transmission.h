//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IEEE802154TRANSMISSION_H
#define __INET_IEEE802154TRANSMISSION_H

#include "inet/physicallayer/wireless/common/base/packetlevel/TransmissionBase.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadio.h"

namespace inet {

namespace physicallayer {

class INET_API Ieee802154Transmission : public TransmissionBase
{
  protected:
    const b headerLength = b(-1);
    const b dataLength = b(-1);
    const IModulation *modulation = nullptr;
    const Hz bandwidth = Hz(NaN);
    const bps bitrate = bps(NaN);

  public:
    Ieee802154Transmission(const IRadio *transmitter, const Packet *packet, const simtime_t startTime, const simtime_t endTime, const simtime_t preambleDuration, const simtime_t headerDuration, const simtime_t dataDuration, const Coord startPosition, const Coord endPosition, const Quaternion startOrientation, const Quaternion endOrientation, const ITransmissionPacketModel *packetModel, const ITransmissionBitModel *bitModel, const ITransmissionSymbolModel *symbolModel, const ITransmissionSampleModel *sampleModel, const ITransmissionAnalogModel *analogModel, b headerLength, b dataLength, const IModulation *modulation, Hz bandwidth, bps bitrate);

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;

    virtual b getHeaderLength() const { return headerLength; }
    virtual b getDataLength() const { return dataLength; }
    virtual const IModulation *getModulation() const { return modulation; }
    virtual const Hz getBandwidth() const { return bandwidth; }
    virtual bps getBitrate() const { return bitrate; }
};

} // namespace physicallayer

} // namespace inet

#endif

