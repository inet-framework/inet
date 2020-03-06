//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_FLATTRANSMITTERBASE_H
#define __INET_FLATTRANSMITTERBASE_H

#include "inet/physicallayer/wireless/common/base/packetlevel/NarrowbandTransmitterBase.h"

namespace inet {

namespace physicallayer {

class INET_API FlatTransmitterBase : public NarrowbandTransmitterBase
{
  protected:
    simtime_t preambleDuration;
    b headerLength;
    bps bitrate;
    double codeRate;
    W power;

  protected:
    virtual void initialize(int stage) override;

    virtual bps computeTransmissionPreambleBitrate(const Packet *packet) const;
    virtual bps computeTransmissionHeaderBitrate(const Packet *packet) const;
    virtual bps computeTransmissionDataBitrate(const Packet *packet) const;
    virtual W computeTransmissionPower(const Packet *packet) const;

  public:
    FlatTransmitterBase();

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;

    virtual b getHeaderLength() const { return headerLength; }
    virtual void setHeaderLength(b headerLength) { this->headerLength = headerLength; }

    virtual bps getBitrate() const { return bitrate; }
    virtual void setBitrate(bps bitrate) { this->bitrate = bitrate; }

    virtual W getMaxPower() const override { return power; }
    virtual W getPower() const { return power; }
    virtual void setPower(W power) { this->power = power; }
};

} // namespace physicallayer

} // namespace inet

#endif

