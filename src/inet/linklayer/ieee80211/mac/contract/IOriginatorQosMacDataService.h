//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_IORIGINATORQOSMACDATASERVICE_H
#define __INET_IORIGINATORQOSMACDATASERVICE_H

#include <memory>

#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/linklayer/ieee80211/mac/contract/IOriginatorMacDataService.h"
#include "inet/linklayer/ieee80211/mac/contract/IFragmentationPolicy.h"
#include "inet/linklayer/ieee80211/mac/contract/IMpduAggregationPolicy.h"

namespace inet {
namespace ieee80211 {

/** Owns a reversible DATA preview. Source packet identities refer to frames still owned by the queue. */
struct INET_API PreparedQosData : public cObject
{
    std::unique_ptr<Packet> packet;
    queueing::IPacketQueue *queue = nullptr;
    std::vector<int64_t> sourceIds;
    bool aggregate = false;

    ~PreparedQosData() override;
    void setPacket(Packet *value);
    Packet *releasePacket();
};

/** Owns queued previews and borrows canonical members. The order is the proposed transmit order. */
struct INET_API PreparedQosBatch
{
    std::vector<std::unique_ptr<PreparedQosData>> queued;
    std::vector<Packet *> members;
};

/** Owns fragment previews. A nonnegative retainedId identifies a numbered source supplied by the caller. */
struct INET_API PreparedQosFragments : public cObject
{
    std::unique_ptr<PreparedQosData> source;
    int64_t retainedId = -1;
    FragmentPartitionStatus status = FragmentPartitionStatus::IMPOSSIBLE;
    std::vector<Packet *> fragments;

    ~PreparedQosFragments() override;
    void addFragment(Packet *packet);
    std::vector<Packet *> releaseFragments();
};

class INET_API IOriginatorQosMacDataService : public IOriginatorMacDataService
{
  public:
    // Preview borrows the frame without extraction or sequence assignment.
    virtual Packet *peekEligibleFrame(queueing::IPacketQueue *queue) const = 0;
    virtual SequenceNumberCyclic getNextSequenceNumber(MacAddress peer, Tid tid) const = 0;
    virtual bool hasAMsduAggregation() const = 0;
    virtual bool hasAMpduAggregation() const = 0;
    virtual std::unique_ptr<PreparedQosBatch> prepareBatch(queueing::IPacketQueue *queue,
            const std::vector<Packet *>& retained, const MpduAggregationLimits& limits) = 0;
    virtual bool isBatchValid(const PreparedQosBatch& batch, simtime_t start) const = 0;
    // Success transfers each new member to the caller; canonical members remain borrowed.
    virtual bool commitPreparedBatch(PreparedQosBatch& batch, std::vector<Packet *>& committed) = 0;
    virtual Packet *buildAggregate(const PreparedQosBatch& batch, const MpduAggregationLimits& limits) const = 0;
    // These QoS DATA methods separate reversible preparation from extraction and sequence assignment.
    virtual std::unique_ptr<PreparedQosData> prepareFrame(queueing::IPacketQueue *queue, bool allowAggregation) = 0;
    virtual bool isPreparationValid(const PreparedQosData& preparation, simtime_t start) const = 0;
    // Success transfers the preview to the caller. Failure retains the preview and all queued sources.
    virtual Packet *commitPreparedFrame(PreparedQosData& preparation) = 0;
    // A null numberedSource selects a queued source. Otherwise the caller supplies a live, never-transmitted source.
    virtual std::unique_ptr<PreparedQosFragments> prepareFragments(queueing::IPacketQueue *queue,
            Packet *numberedSource, int maxMpduBytes) = 0;
    virtual bool isFragmentPreparationValid(const PreparedQosFragments& preparation,
            Packet *numberedSource, simtime_t start) const = 0;
    // Success transfers previews. The caller replaces a numbered source through InProgressFrames.
    virtual bool commitPreparedFragments(PreparedQosFragments& preparation,
            Packet *numberedSource, std::vector<Packet *>& committed) = 0;
};

} // namespace ieee80211
} // namespace inet

#endif
