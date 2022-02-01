//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_RECIPIENTBLOCKACKAGREEMENTPOLICY_H
#define __INET_RECIPIENTBLOCKACKAGREEMENTPOLICY_H

#include "inet/linklayer/ieee80211/mac/blockack/RecipientBlockAckAgreement.h"
#include "inet/linklayer/ieee80211/mac/contract/IRecipientBlockAckAgreementPolicy.h"

namespace inet {
namespace ieee80211 {

class INET_API RecipientBlockAckAgreementPolicy : public cSimpleModule, public IRecipientBlockAckAgreementPolicy
{
  protected:
    int maximumAllowedBufferSize = -1;
    bool isAMsduSupported = false;
    bool isDelayedBlockAckPolicySupported = false;
    simtime_t blockAckTimeoutValue = -1;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;

  public:
    virtual bool isAddbaReqAccepted(const Ptr<const Ieee80211AddbaRequest>& addbaReq) override;
    virtual bool isDelbaAccepted(const Ptr<const Ieee80211Delba>& delba) override;

    virtual simtime_t getBlockAckTimeoutValue() const override { return blockAckTimeoutValue; }
    virtual bool aMsduSupported() const override { return isAMsduSupported; }
    virtual bool delayedBlockAckPolicySupported() const override { return isDelayedBlockAckPolicySupported; }
    virtual int getMaximumAllowedBufferSize() const override { return maximumAllowedBufferSize; }
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif

