//
// Copyright (C) 2013 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_FLATTRANSMISSIONBASE_H
#define __INET_FLATTRANSMISSIONBASE_H

#include "inet/physicallayer/base/TransmissionBase.h"
#include "inet/physicallayer/contract/IModulation.h"

namespace inet {

namespace physicallayer {

class INET_API FlatTransmissionBase : public TransmissionBase
{
  protected:
    const IModulation *modulation;
    const int headerBitLength;
    const int payloadBitLength;
    const Hz carrierFrequency;
    const Hz bandwidth;
    const bps bitrate;

  public:
    FlatTransmissionBase(const IRadio *transmitter, const cPacket *macFrame, const simtime_t startTime, const simtime_t endTime, const Coord startPosition, const Coord endPosition, const EulerAngles startOrientation, const EulerAngles endOrientation, const IModulation *modulation, int headerBitLength, int payloadBitLength, Hz carrierFrequency, Hz bandwidth, bps bitrate) :
        TransmissionBase(transmitter, macFrame, startTime, endTime, startPosition, endPosition, startOrientation, endOrientation),
        modulation(modulation),
        headerBitLength(headerBitLength),
        payloadBitLength(payloadBitLength),
        carrierFrequency(carrierFrequency),
        bandwidth(bandwidth),
        bitrate(bitrate)
    {}

    virtual const IModulation *getModulation() const { return modulation; }
    virtual int getHeaderBitLength() const { return headerBitLength; }
    virtual int getPayloadBitLength() const { return payloadBitLength; }
    virtual Hz getCarrierFrequency() const { return carrierFrequency; }
    virtual Hz getBandwidth() const { return bandwidth; }
    virtual bps getBitrate() const { return bitrate; }
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_FLATTRANSMISSIONBASE_H

