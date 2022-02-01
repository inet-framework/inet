//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IEEE80211TRANSMISSIONBASE_H
#define __INET_IEEE80211TRANSMISSIONBASE_H

#include "inet/physicallayer/wireless/ieee80211/mode/IIeee80211Mode.h"
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211Channel.h"

namespace inet {

namespace physicallayer {

class INET_API Ieee80211TransmissionBase : public IPrintableObject
{
  protected:
    const IIeee80211Mode *mode;
    const Ieee80211Channel *channel;

  public:
    Ieee80211TransmissionBase(const IIeee80211Mode *mode, const Ieee80211Channel *channel);

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;

    virtual const IIeee80211Mode *getMode() const { return mode; }
    virtual const Ieee80211Channel *getChannel() const { return channel; }
};

} // namespace physicallayer

} // namespace inet

#endif

