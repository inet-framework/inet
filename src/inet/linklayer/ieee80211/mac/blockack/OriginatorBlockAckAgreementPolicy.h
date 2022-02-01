//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ORIGINATORBLOCKACKAGREEMENTPOLICY_H
#define __INET_ORIGINATORBLOCKACKAGREEMENTPOLICY_H

#include "inet/linklayer/ieee80211/mac/blockack/OriginatorBlockAckAgreementHandler.h"
#include "inet/linklayer/ieee80211/mac/common/ModeSetListener.h"
#include "inet/linklayer/ieee80211/mac/contract/IOriginatorQoSAckPolicy.h"

namespace inet {
namespace ieee80211 {

class INET_API OriginatorBlockAckAgreementPolicy : public ModeSetListener, public IOriginatorBlockAckAgreementPolicy
{
  protected:
    IOriginatorQoSAckPolicy *ackPolicy = nullptr;

    int blockAckReqThreshold = -1;
    bool delayedAckPolicySupported = false;
    bool aMsduSupported = false;
    int maximumAllowedBufferSize = -1;
    simtime_t blockAckTimeoutValue = -1;
    simtime_t addbaFailureTimeout = -1;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;

  public:
    virtual bool isAddbaReqNeeded(Packet *packet, const Ptr<const Ieee80211DataHeader>& header) override;
    virtual bool isAddbaReqAccepted(const Ptr<const Ieee80211AddbaResponse>& addbaResp, OriginatorBlockAckAgreement *agreement) override;
    virtual bool isDelbaAccepted(const Ptr<const Ieee80211Delba>& delba) override;

    virtual simtime_t computeAddbaFailureTimeout() const override;

    virtual bool isMsduSupported() const override { return aMsduSupported; }
    virtual simtime_t getBlockAckTimeoutValue() const override { return blockAckTimeoutValue; }
    virtual bool isDelayedAckPolicySupported() const override { return delayedAckPolicySupported; }
    virtual int getMaximumAllowedBufferSize() const override { return maximumAllowedBufferSize; }
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif

