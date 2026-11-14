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

class INET_API Ieee80211Channel : public IPrintableObject
{
  protected:
    const IIeee80211Band *band;
    int channelNumber;
    Hz allocationWidth = Hz(NaN);

  public:
    Ieee80211Channel(const IIeee80211Band *band, int channelNumber);
    // Opt-in contiguous 5 GHz allocation; channelNumber remains a band index.
    Ieee80211Channel(const IIeee80211Band *band, int channelNumber, Hz allocationWidth);

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;

    virtual const IIeee80211Band *getBand() const { return band; }
    virtual int getChannelNumber() const { return channelNumber; }
    virtual Hz getCenterFrequency() const { return band->getCenterFrequency(channelNumber); }
    bool hasAllocation() const { return !std::isnan(allocationWidth.get()); }
    Hz getAllocationWidth() const { return allocationWidth; }
    int getPrimaryChannel() const { return band->getStandardChannelNumber(channelNumber); }
    int getCenterChannel(Hz width) const;
    Hz getCenterFrequency(Hz width) const;
    bool containsPrimaryAllocation(Hz center, Hz width) const;
};

} // namespace physicallayer

} // namespace inet

#endif

