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
#include "inet/linklayer/ieee80211/mac/contract/IOriginatorQosMacDataService.h"
#include "inet/linklayer/ieee80211/mac/contract/ISequenceNumberAssignment.h"
#include "inet/linklayer/ieee80211/mac/lifetime/EdcaTransmitLifetimeHandler.h"

namespace inet {
namespace ieee80211 {

class INET_API OriginatorQosMacDataService : public IOriginatorQosMacDataService, public SimpleModule
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
    std::function<bool(Packet *)> extractionEligible;
    std::function<bool(Packet *)> aMsduPermitted;
    std::function<bool(Packet *)> fragmentationPermitted;
    int64_t nextDataAllocationIndex = 0;

  protected:
    virtual void initialize() override;

    virtual void assignSequenceNumber(const Ptr<Ieee80211DataOrMgmtHeader>& header);
    virtual std::vector<Packet *> *fragmentIfNeeded(Packet *frame);
    virtual Packet *aMsduAggregateIfNeeded(queueing::IPacketQueue *pendingQueue, Packet *selected);
    virtual Packet *aMpduAggregateIfNeeded(std::vector<Packet *> *fragments);
    bool isPreparationValidForSequence(const PreparedQosData& preparation, simtime_t start, SequenceNumberCyclic sequence) const;
    Packet *commitValidatedFrame(PreparedQosData& preparation);
    void transferFragmentMetadata(PreparedQosFragments& preparation, Packet *source,
            SequenceNumberCyclic sequence, int64_t allocation, simtime_t deadline);

  public:
    virtual std::unique_ptr<PreparedQosFragments> prepareFragments(queueing::IPacketQueue *queue,
            Packet *numberedSource, int maxMpduBytes) override;
    virtual bool isFragmentPreparationValid(const PreparedQosFragments& preparation,
            Packet *numberedSource, simtime_t start) const override;
    virtual bool commitPreparedFragments(PreparedQosFragments& preparation,
            Packet *numberedSource, std::vector<Packet *>& committed) override;
    void setFragmentationPermission(std::function<bool(Packet *)> permission) { fragmentationPermitted = permission; }
    void setEligibility(std::function<bool(Packet *)> eligible, std::function<bool(Packet *)> permission) {
        extractionEligible = eligible;
        aMsduPermitted = permission;
    }
    virtual Packet *peekEligibleFrame(queueing::IPacketQueue *queue) const override;
    virtual SequenceNumberCyclic getNextSequenceNumber(MacAddress peer, Tid tid) const override;
    virtual bool hasAMsduAggregation() const override { return aMsduAggregationPolicy != nullptr; }
    virtual bool hasAMpduAggregation() const override { return aMpduAggregationPolicy && aMpduAggregationPolicy->isEnabled(); }
    virtual std::unique_ptr<PreparedQosBatch> prepareBatch(queueing::IPacketQueue *queue,
            const std::vector<Packet *>& retained, const MpduAggregationLimits& limits) override;
    virtual bool isBatchValid(const PreparedQosBatch& batch, simtime_t start) const override;
    virtual bool commitPreparedBatch(PreparedQosBatch& batch, std::vector<Packet *>& committed) override;
    virtual Packet *buildAggregate(const PreparedQosBatch& batch, const MpduAggregationLimits& limits) const override;
    virtual std::unique_ptr<PreparedQosData> prepareFrame(queueing::IPacketQueue *queue, bool allowAggregation) override;
    virtual bool isPreparationValid(const PreparedQosData& preparation, simtime_t start) const override;
    virtual Packet *commitPreparedFrame(PreparedQosData& preparation) override;
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

