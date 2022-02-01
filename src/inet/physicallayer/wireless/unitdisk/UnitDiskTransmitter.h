//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_UNITDISKTRANSMITTER_H
#define __INET_UNITDISKTRANSMITTER_H

#include "inet/physicallayer/wireless/common/base/packetlevel/TransmitterBase.h"

namespace inet {

namespace physicallayer {

/**
 * Implements the UnitDiskTransmitter model, see the NED file for details.
 */
class INET_API UnitDiskTransmitter : public TransmitterBase
{
  protected:
    simtime_t preambleDuration;
    b headerLength;
    bps bitrate;
    m communicationRange;
    m interferenceRange;
    m detectionRange;

  protected:
    virtual void initialize(int stage) override;

  public:
    UnitDiskTransmitter();

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;
    virtual const ITransmission *createTransmission(const IRadio *radio, const Packet *packet, const simtime_t startTime) const override;
    virtual simtime_t getPreambleDuration() const { return preambleDuration; }
    virtual b getHeaderLength() const { return headerLength; }
    virtual bps getBitrate() const { return bitrate; }
    virtual m getMaxCommunicationRange() const override { return communicationRange; }
    virtual m getMaxInterferenceRange() const override { return interferenceRange; }
};

} // namespace physicallayer

} // namespace inet

#endif

