//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IEEE80211TRANSMITTER_H
#define __INET_IEEE80211TRANSMITTER_H

#include "inet/physicallayer/wireless/common/base/packetlevel/FlatTransmitterBase.h"
#include "inet/physicallayer/wireless/ieee80211/mode/IIeee80211Mode.h"
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211Band.h"
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211Channel.h"
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211ModeSet.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211ControlInfo_m.h"

namespace inet {

namespace physicallayer {

class INET_API Ieee80211Transmitter : public FlatTransmitterBase
{
  protected:
    const Ieee80211ModeSet *modeSet;
    const IIeee80211Mode *mode;
    const IIeee80211Band *band;
    const Ieee80211Channel *channel;

  protected:
    virtual void initialize(int stage) override;

  public:
    Ieee80211Transmitter();
    virtual ~Ieee80211Transmitter();

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;

    virtual const IIeee80211Mode *computeTransmissionMode(const Packet *packet) const;
    virtual const Ieee80211Channel *computeTransmissionChannel(const Packet *packet) const;

    virtual void setModeSet(const Ieee80211ModeSet *modeSet);
    virtual void setMode(const IIeee80211Mode *mode);
    virtual void setBand(const IIeee80211Band *band);
    virtual void setChannel(const Ieee80211Channel *channel);
    virtual void setChannelNumber(int channelNumber);

    virtual const ITransmission *createTransmission(const IRadio *radio, const Packet *packet, simtime_t startTime) const override;
};

} // namespace physicallayer

} // namespace inet

#endif

