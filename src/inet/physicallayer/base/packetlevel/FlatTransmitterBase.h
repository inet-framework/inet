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

#ifndef __INET_FLATTRANSMITTERBASE_H
#define __INET_FLATTRANSMITTERBASE_H

#include "inet/physicallayer/base/packetlevel/NarrowbandTransmitterBase.h"

namespace inet {

namespace physicallayer {

class INET_API FlatTransmitterBase : public NarrowbandTransmitterBase
{
  protected:
    simtime_t preambleDuration;
    int headerBitLength;
    bps bitrate;
    W power;

  protected:
    virtual void initialize(int stage) override;

  public:
    FlatTransmitterBase();

    virtual std::ostream& printToStream(std::ostream& stream, int level) const override;

    virtual int getHeaderBitLength() const { return headerBitLength; }
    virtual void setHeaderBitLength(int headerBitLength) { this->headerBitLength = headerBitLength; }

    virtual bps getBitrate() const { return bitrate; }
    virtual void setBitrate(bps bitrate) { this->bitrate = bitrate; }

    virtual W getMaxPower() const override { return power; }
    virtual W getPower() const { return power; }
    virtual void setPower(W power) { this->power = power; }
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_FLATTRANSMITTERBASE_H

