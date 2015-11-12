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

#include "inet/physicallayer/base/packetlevel/NarrowbandTransmissionBase.h"

namespace inet {

namespace physicallayer {

class INET_API FlatTransmissionBase : public NarrowbandTransmissionBase
{
  protected:
    const int headerBitLength;
    const int dataBitLength;
    const bps bitrate;

  public:
    FlatTransmissionBase(const IRadio *transmitter, const cPacket *macFrame, const simtime_t startTime, const simtime_t endTime, const simtime_t preambleDuration, const simtime_t headerDuration, const simtime_t dataDuration, const Coord startPosition, const Coord endPosition, const EulerAngles startOrientation, const EulerAngles endOrientation, int headerBitLength, int payloadBitLength, bps bitrate, const IModulation *modulation, Hz carrierFrequency, Hz bandwidth);

    virtual std::ostream& printToStream(std::ostream& stream, int level) const override;

    virtual int getHeaderBitLength() const { return headerBitLength; }
    virtual int getDataBitLength() const { return dataBitLength; }
    virtual bps getBitrate() const { return bitrate; }
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_FLATTRANSMISSIONBASE_H

