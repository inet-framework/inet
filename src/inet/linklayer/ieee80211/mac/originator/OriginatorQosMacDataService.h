//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ORIGINATORQOSMACDATASERVICE_H
#define __INET_ORIGINATORQOSMACDATASERVICE_H

#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/linklayer/ieee80211/mac/contract/IFragmentation.h"
#include "inet/linklayer/ieee80211/mac/contract/IFragmentationPolicy.h"
#include "inet/linklayer/ieee80211/mac/contract/IMpduAggregation.h"
#include "inet/linklayer/ieee80211/mac/contract/IMpduAggregationPolicy.h"
#include "inet/linklayer/ieee80211/mac/contract/IMsduAggregation.h"
#include "inet/linklayer/ieee80211/mac/contract/IMsduAggregationPolicy.h"
#include "inet/linklayer/ieee80211/mac/contract/IOriginatorMacDataService.h"
#include "inet/linklayer/ieee80211/mac/contract/ISequenceNumberAssignment.h"

namespace inet {
namespace ieee80211 {

class INET_API OriginatorQosMacDataService : public IOriginatorMacDataService, public cSimpleModule
{
  protected:
    // Figure 5-1—MAC data plane architecture
//    MsduRateLimiting *msduRateLimiting = nullptr;
    ISequenceNumberAssignment *sequenceNumberAssignment = nullptr;
//    MsduIntegrityAndProtection *msduIntegrityAndProtection = nullptr;
//    MpduEncryptionAndIntegrity *mpduEncryptionAndIntegrity = nullptr;
//    MpduHeaderPlusCrc *mpduHeaderPlusCrc = nullptr;
    IFragmentationPolicy *fragmentationPolicy = nullptr;
    IFragmentation *fragmentation = nullptr;
    IMsduAggregationPolicy *aMsduAggregationPolicy = nullptr;
    IMsduAggregation *aMsduAggregation = nullptr;
//    PsDeferQueueing *psDeferQueueing = nullptr;
    IMpduAggregationPolicy *aMpduAggregationPolicy = nullptr;
    IMpduAggregation *aMpduAggregation = nullptr;

  protected:
    virtual void initialize() override;

    virtual void assignSequenceNumber(const Ptr<Ieee80211DataOrMgmtHeader>& header);
    virtual std::vector<Packet *> *fragmentIfNeeded(Packet *frame);
    virtual Packet *aMsduAggregateIfNeeded(queueing::IPacketQueue *pendingQueue);
    virtual Packet *aMpduAggregateIfNeeded(std::vector<Packet *> *fragments);

  public:
    virtual ~OriginatorQosMacDataService();

    virtual std::vector<Packet *> *extractFramesToTransmit(queueing::IPacketQueue *pendingQueue) override;
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif

