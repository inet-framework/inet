//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IEEE80211UNITDISKTRANSMITTER_H
#define __INET_IEEE80211UNITDISKTRANSMITTER_H

#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211TransmitterBase.h"

namespace inet {

namespace physicallayer {

class INET_API Ieee80211UnitDiskTransmitter : public Ieee80211TransmitterBase
{
  protected:
    m communicationRange = m(NaN);
    m interferenceRange = m(NaN);
    m detectionRange = m(NaN);

  protected:
    virtual void initialize(int stage) override;

  public:
    Ieee80211UnitDiskTransmitter();
    virtual m getMaxCommunicationRange() const override { return communicationRange; }
    virtual m getMaxInterferenceRange() const override { return interferenceRange; }
    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;

    virtual const ITransmission *createTransmission(const IRadio *radio, const Packet *packet, simtime_t startTime) const override;
};

} // namespace physicallayer

} // namespace inet

#endif

