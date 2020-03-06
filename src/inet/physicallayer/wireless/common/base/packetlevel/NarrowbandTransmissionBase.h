//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_NARROWBANDTRANSMISSIONBASE_H
#define __INET_NARROWBANDTRANSMISSIONBASE_H

#include "inet/physicallayer/wireless/common/base/packetlevel/TransmissionBase.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IModulation.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadioSignal.h"

namespace inet {

namespace physicallayer {

class INET_API NarrowbandTransmissionBase : public TransmissionBase, public virtual INarrowbandSignal
{
  protected:
    const IModulation *modulation;
    const simtime_t symbolTime;
    const Hz centerFrequency;
    const Hz bandwidth;

  public:
    NarrowbandTransmissionBase(const IRadio *transmitter, const Packet *packet, const simtime_t startTime, const simtime_t endTime, const simtime_t preambleDuration, const simtime_t headerDuration, const simtime_t dataDuration, const Coord startPosition, const Coord endPosition, const Quaternion startOrientation, const Quaternion endOrientation, const IModulation *modulation, const simtime_t symbolTime, Hz centerFrequency, Hz bandwidth);

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;

    virtual const IModulation *getModulation() const { return modulation; }
    virtual simtime_t getSymbolTime() const { return symbolTime; }
    virtual Hz getCenterFrequency() const override { return centerFrequency; }
    virtual Hz getBandwidth() const override { return bandwidth; }
};

} // namespace physicallayer

} // namespace inet

#endif

