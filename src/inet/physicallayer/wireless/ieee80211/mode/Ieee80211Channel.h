//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IEEE80211CHANNEL_H
#define __INET_IEEE80211CHANNEL_H

#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211Band.h"

namespace inet {

namespace physicallayer {

// IEEE Std 802.11-2024, Table 9-134.
enum Ieee80211SecondaryChannelOffset {
    IEEE80211_SECONDARY_CHANNEL_NONE = 0,
    IEEE80211_SECONDARY_CHANNEL_ABOVE = 1,
    IEEE80211_SECONDARY_CHANNEL_BELOW = 3,
};

class INET_API Ieee80211Channel : public IPrintableObject
{
  protected:
    const IIeee80211Band *band;
    int channelNumber;
    Ieee80211SecondaryChannelOffset secondaryChannelOffset;

  public:
    Ieee80211Channel(const IIeee80211Band *band, int channelNumber);
    Ieee80211Channel(const IIeee80211Band *band, int channelNumber, Ieee80211SecondaryChannelOffset secondaryChannelOffset);

    static Ieee80211SecondaryChannelOffset parseSecondaryChannelOffset(const char *text);
    static const char *getSecondaryChannelOffsetName(Ieee80211SecondaryChannelOffset offset);

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;

    virtual const IIeee80211Band *getBand() const { return band; }
    virtual int getChannelNumber() const { return channelNumber; }
    virtual Ieee80211SecondaryChannelOffset getSecondaryChannelOffset() const { return secondaryChannelOffset; }
    virtual int getSecondaryChannelNumber() const;
    virtual Hz getCenterFrequency() const { return band->getCenterFrequency(channelNumber); }
    virtual Hz getSecondaryCenterFrequency() const;
    virtual Hz getBondedCenterFrequency() const;
};

} // namespace physicallayer

} // namespace inet

#endif

