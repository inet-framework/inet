//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_IORIGINATORQOSMACDATASERVICE_H
#define __INET_IORIGINATORQOSMACDATASERVICE_H

#include <functional>
#include <memory>

#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/linklayer/ieee80211/mac/contract/IOriginatorMacDataService.h"
#include "inet/linklayer/ieee80211/mac/contract/IMpduAggregation.h"

namespace inet {
namespace ieee80211 {

/** Owns a reversible DATA preview. Source packet identities refer to frames still owned by the queue. */
struct INET_API PreparedQosData : public cObject
{
    std::unique_ptr<Packet> packet;
    queueing::IPacketQueue *queue = nullptr;
    std::vector<int64_t> sourceIds;
    bool aggregate = false;
    int sequenceOffset = 0; // Reversible position among fresh same-peer/TID MPDUs.

    ~PreparedQosData() override;
    void setPacket(Packet *value);
    Packet *releasePacket();
};

class INET_API IOriginatorQosMacDataService : public IOriginatorMacDataService
{
  public:
    // HCF owns the permission decision. An empty callback permits the configured fragmentation policy.
    // The service retains the callback and queries it before fragmentation, without packet ownership transfer.
    virtual void setFragmentationPermission(std::function<bool(Packet *)> permission) = 0;
    // Preview borrows the frame without extraction or sequence assignment.
    virtual Packet *peekEligibleFrame(queueing::IPacketQueue *queue) const = 0;
    virtual SequenceNumberCyclic getNextSequenceNumber(MacAddress peer, Tid tid) const = 0;
    virtual bool hasAMsduAggregation() const = 0;
    virtual bool hasAMpduAggregation() const = 0;
    // These QoS DATA methods separate reversible preparation from extraction and sequence assignment.
    virtual std::unique_ptr<PreparedQosData> prepareFrame(queueing::IPacketQueue *queue, bool allowAggregation) = 0;
    virtual std::vector<std::unique_ptr<PreparedQosData>> prepareFrames(queueing::IPacketQueue *queue,
        bool allowMsduAggregation, int maxMembers, B memberLimit) = 0;
    // Applies the configured policy to ordered borrowed MPDU candidates.
    virtual std::vector<Packet *> selectAmpduFrames(const std::vector<Packet *>& candidates) = 0;
    // Borrows retained originals/previews and returns an owned encoded copy.
    virtual Packet *buildAmpdu(const std::vector<Packet *>& members, const MpduAggregationParameters& parameters) = 0;
    virtual bool isPreparationValid(const PreparedQosData& preparation, simtime_t start) const = 0;
    // Success transfers the preview to the caller. Failure retains the preview and all queued sources.
    virtual Packet *commitPreparedFrame(PreparedQosData& preparation) = 0;
};

} // namespace ieee80211
} // namespace inet

#endif
