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

class INET_API IOriginatorQosMacDataService : public IOriginatorMacDataService
{
  public:
    // Preview borrows the frame without extraction or sequence assignment.
    virtual Packet *peekEligibleFrame(queueing::IPacketQueue *queue) const = 0;
    virtual SequenceNumberCyclic getNextSequenceNumber(MacAddress peer, Tid tid) const = 0;
    virtual bool hasAMsduAggregation() const = 0;
    // These QoS DATA methods separate reversible preparation from extraction and sequence assignment.
    virtual std::unique_ptr<PreparedQosData> prepareFrame(queueing::IPacketQueue *queue, bool allowAggregation) = 0;
    virtual bool isPreparationValid(const PreparedQosData& preparation, simtime_t start) const = 0;
    // Success transfers the preview to the caller. Failure retains the preview and all queued sources.
    virtual Packet *commitPreparedFrame(PreparedQosData& preparation) = 0;
};

} // namespace ieee80211
} // namespace inet

#endif
