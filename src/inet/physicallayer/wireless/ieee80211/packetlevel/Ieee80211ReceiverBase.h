//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IEEE80211RECEIVERBASE_H
#define __INET_IEEE80211RECEIVERBASE_H

#include "inet/physicallayer/wireless/common/base/packetlevel/FlatReceiverBase.h"
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211Band.h"
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211Channel.h"
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211ModeSet.h"

namespace inet {
namespace physicallayer {

class INET_API Ieee80211ReceiverBase : public FlatReceiverBase
{
  protected:
    const Ieee80211ModeSet *modeSet;
    const IIeee80211Band *band;
    const Ieee80211Channel *channel;

  protected:
    virtual void initialize(int stage) override;

  public:
    Ieee80211ReceiverBase();
    virtual ~Ieee80211ReceiverBase();

    virtual const IReceptionResult *computeReceptionResult(const IListening *listening, const IReception *reception, const IInterference *interference, const ISnir *snir, const std::vector<const IReceptionDecision *> *decisions) const override;

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;

    virtual void setModeSet(const Ieee80211ModeSet *modeSet);
    virtual void setBand(const IIeee80211Band *band);
    virtual void setChannel(const Ieee80211Channel *channel);
    virtual void setChannelNumber(int channelNumber);
};

} // namespace physicallayer
} // namespace inet

#endif

