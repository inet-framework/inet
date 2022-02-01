//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_RECIPIENTQOSACKPOLICY_H
#define __INET_RECIPIENTQOSACKPOLICY_H

#include "inet/linklayer/ieee80211/mac/common/ModeSetListener.h"
#include "inet/linklayer/ieee80211/mac/contract/IQosRateSelection.h"
#include "inet/linklayer/ieee80211/mac/contract/IRecipientAckPolicy.h"
#include "inet/linklayer/ieee80211/mac/contract/IRecipientQosAckPolicy.h"

namespace inet {
namespace ieee80211 {

class INET_API RecipientQosAckPolicy : public ModeSetListener, public IRecipientAckPolicy, public IRecipientQosAckPolicy
{
  protected:
    IQosRateSelection *rateSelection = nullptr;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;

    simtime_t computeBasicBlockAckDuration(Packet *packet, const Ptr<const Ieee80211BlockAckReq>& blockAckReq) const;
    simtime_t computeAckDuration(Packet *packet, const Ptr<const Ieee80211DataOrMgmtHeader>& dataOrMgmtHeader) const;

  public:
    virtual bool isAckNeeded(const Ptr<const Ieee80211DataOrMgmtHeader>& header) const override;
    virtual bool isBlockAckNeeded(const Ptr<const Ieee80211BlockAckReq>& blockAckReq, RecipientBlockAckAgreement *agreement) const override;

    virtual simtime_t computeAckDurationField(Packet *packet, const Ptr<const Ieee80211DataOrMgmtHeader>& header) const override;
    virtual simtime_t computeBasicBlockAckDurationField(Packet *packet, const Ptr<const Ieee80211BasicBlockAckReq>& basicBlockAckReq) const override;
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif

