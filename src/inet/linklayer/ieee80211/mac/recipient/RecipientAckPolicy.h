//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_RECIPIENTACKPOLICY_H
#define __INET_RECIPIENTACKPOLICY_H

#include "inet/linklayer/ieee80211/mac/common/ModeSetListener.h"
#include "inet/linklayer/ieee80211/mac/contract/IRateSelection.h"
#include "inet/linklayer/ieee80211/mac/contract/IRecipientAckPolicy.h"

namespace inet {
namespace ieee80211 {

class INET_API RecipientAckPolicy : public ModeSetListener, public IRecipientAckPolicy
{
  protected:
    IRateSelection *rateSelection = nullptr;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;

    simtime_t computeAckDuration(Packet *dataOrMgmtPacket, const Ptr<const Ieee80211DataOrMgmtHeader>& dataOrMgmtHeader) const;

  public:
    virtual bool isAckNeeded(const Ptr<const Ieee80211DataOrMgmtHeader>& header) const override;

    virtual simtime_t computeAckDurationField(Packet *packet, const Ptr<const Ieee80211DataOrMgmtHeader>& header) const override;
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif

