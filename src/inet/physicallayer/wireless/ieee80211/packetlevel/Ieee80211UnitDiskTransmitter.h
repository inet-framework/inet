//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IEEE80211UNITDISKTRANSMITTER_H
#define __INET_IEEE80211UNITDISKTRANSMITTER_H

#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211TransmitterBase.h"
#include "inet/physicallayer/wireless/common/analogmodel/packetlevel/UnitDiskTransmitterAnalogModel.h"

namespace inet {

namespace physicallayer {

class INET_API Ieee80211UnitDiskTransmitter : public Ieee80211TransmitterBase
{
  public:
    Ieee80211UnitDiskTransmitter();
    virtual m getMaxCommunicationRange() const override {
        if (auto analogModel = dynamic_cast<UnitDiskTransmitterAnalogModel *>(getAnalogModel()))
            return analogModel->getCommunicationRange();
        else
            return Ieee80211TransmitterBase::getMaxCommunicationRange();
    }

    virtual m getMaxInterferenceRange() const override {
        if (auto analogModel = dynamic_cast<UnitDiskTransmitterAnalogModel *>(getAnalogModel()))
            return analogModel->getInterferenceRange();
        else
            return Ieee80211TransmitterBase::getMaxInterferenceRange();
    }
    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;

    virtual const ITransmission *createTransmission(const IRadio *radio, const Packet *packet, simtime_t startTime) const override;
};

} // namespace physicallayer

} // namespace inet

#endif

