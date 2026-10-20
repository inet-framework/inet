//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ORIGINATORQOSMACDATASERVICE_H
#define __INET_ORIGINATORQOSMACDATASERVICE_H

#include "inet/common/SimpleModule.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/linklayer/ieee80211/mac/contract/IFragmentation.h"
#include "inet/linklayer/ieee80211/mac/contract/IFragmentationPolicy.h"
#include "inet/linklayer/ieee80211/mac/contract/IMpduAggregation.h"
#include "inet/linklayer/ieee80211/mac/contract/IMpduAggregationPolicy.h"
#include "inet/linklayer/ieee80211/mac/contract/IMsduAggregation.h"
#include "inet/linklayer/ieee80211/mac/contract/IMsduAggregationPolicy.h"
#include "inet/linklayer/ieee80211/mac/contract/IOriginatorMacDataService.h"
#include "inet/linklayer/ieee80211/mac/contract/ISequenceNumberAssignment.h"
#include "inet/linklayer/ieee80211/mac/lifetime/EdcaTransmitLifetimeHandler.h"

namespace inet {
namespace ieee80211 {

class INET_API OriginatorQosMacDataService : public IOriginatorMacDataService, public SimpleModule
{
  protected:
    // Figure 5-1—MAC data plane architecture
//    MsduRateLimiting *msduRateLimiting = nullptr;
    ISequenceNumberAssignment *sequenceNumberAssignment = nullptr;
//    MsduIntegrityAndProtection *msduIntegrityAndProtection = nullptr;
//    MpduEncryptionAndIntegrity *mpduEncryptionAndIntegrity = nullptr;
//    MpduHeaderPlusFcs *mpduHeaderPlusFcs = nullptr;
    IFragmentationPolicy *fragmentationPolicy = nullptr;
    IFragmentation *fragmentation = nullptr;
    IMsduAggregationPolicy *aMsduAggregationPolicy = nullptr;
    IMsduAggregation *aMsduAggregation = nullptr;
//    PsDeferQueueing *psDeferQueueing = nullptr;
    IMpduAggregationPolicy *aMpduAggregationPolicy = nullptr;
    IMpduAggregation *aMpduAggregation = nullptr;
    EdcaTransmitLifetimeHandler *lifetimeHandler = nullptr; // Borrowed from EDCA.
    std::function<void()> expireFrames;
    std::function<bool(const Packet *)> extractionEligible;
    std::function<bool(Packet *)> aMsduPermitted;

  protected:
    virtual void initialize() override;

    virtual void assignSequenceNumber(const Ptr<Ieee80211DataOrMgmtHeader>& header);
    virtual std::vector<Packet *> *fragmentIfNeeded(Packet *frame);
    virtual Packet *aMsduAggregateIfNeeded(queueing::IPacketQueue *pendingQueue, Packet *selected);
    virtual Packet *aMpduAggregateIfNeeded(std::vector<Packet *> *fragments);

  public:
    void setEligibility(std::function<bool(const Packet *)> eligible, std::function<bool(Packet *)> permission) {
        extractionEligible = eligible;
        aMsduPermitted = permission;
    }
    Packet *peekEligibleFrame(queueing::IPacketQueue *queue) const;
    SequenceNumberCyclic getNextSequenceNumber(MacAddress peer, Tid tid) const;
    bool hasAMsduAggregation() const { return aMsduAggregationPolicy != nullptr; }
    virtual ~OriginatorQosMacDataService();
    void setLifetimeHandler(EdcaTransmitLifetimeHandler *handler, std::function<void()> expiration) {
        lifetimeHandler = handler;
        expireFrames = expiration;
    }

    virtual std::vector<Packet *> *extractFramesToTransmit(queueing::IPacketQueue *pendingQueue) override;
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif

