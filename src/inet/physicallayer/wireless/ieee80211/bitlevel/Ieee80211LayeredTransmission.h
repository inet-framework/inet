//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IEEE80211LAYEREDTRANSMISSION_H
#define __INET_IEEE80211LAYEREDTRANSMISSION_H

#include "inet/physicallayer/wireless/common/analogmodel/common/LayeredTransmission.h"
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211Channel.h"
#include "inet/physicallayer/wireless/ieee80211/mode/IIeee80211Mode.h"

namespace inet {

namespace physicallayer {

class INET_API Ieee80211LayeredTransmission : public LayeredTransmission
{
  protected:
    const IIeee80211Mode *mode = nullptr;
    const Ieee80211Channel *channel = nullptr;

  public:
    Ieee80211LayeredTransmission(const ITransmissionPacketModel *packetModel, const ITransmissionBitModel *bitModel, const ITransmissionSymbolModel *symbolModel, const ITransmissionSampleModel *sampleModel, const ITransmissionAnalogModel *analogModel, const IRadio *transmitter, const Packet *packet, const simtime_t startTime, const simtime_t endTime, const simtime_t preambleDuration, const simtime_t headerDuration, const simtime_t dataDuration, const Coord& startPosition, const Coord& endPosition, const Quaternion& startOrientation, const Quaternion& endOrientation, const IIeee80211Mode *mode, const Ieee80211Channel *channel);

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;

    virtual const IIeee80211Mode *getMode() const { return mode; }
    virtual const Ieee80211Channel *getChannel() const { return channel; }
};

} // namespace physicallayer

} // namespace inet

#endif

