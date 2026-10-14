//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_EDCATRANSMITLIFETIMEHANDLER_H
#define __INET_EDCATRANSMITLIFETIMEHANDLER_H

#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/linklayer/ieee80211/mac/common/AccessCategory.h"
#include "inet/linklayer/ieee80211/mac/common/Ieee80211Defs.h"
#include <functional>

#include "inet/common/packet/Packet.h"

namespace inet {
namespace ieee80211 {

class INET_API EdcaTransmitLifetimeHandler
{
  protected:
    simtime_t msduLifetime[4];
    // Local packet identities distinguish peers before sequence assignment.
    using DeadlineQueue = std::set<std::pair<simtime_t, int64_t>>;
    struct FrameLifetime {
        DeadlineQueue::iterator deadline;
        Packet *packet = nullptr; // Borrowed until disposal or frame replacement unregisters it.
    };
    DeadlineQueue deadlines;
    // The identity index refers to the single stored deadline value.
    std::map<int64_t, FrameLifetime> frames;
    std::function<void()> deadlineChanged;

    void addDeadline(Packet *packet, simtime_t deadline);
    bool eraseDeadline(int64_t packetId);

  public:
    EdcaTransmitLifetimeHandler(simtime_t bkLifetime, simtime_t beLifetime, simtime_t viLifetime, simtime_t voLifetime);
    EdcaTransmitLifetimeHandler(const EdcaTransmitLifetimeHandler&) = delete;
    EdcaTransmitLifetimeHandler& operator=(const EdcaTransmitLifetimeHandler&) = delete;

    void setDeadlineChangedCallback(std::function<void()> callback);
    void registerFrame(Packet *packet, AccessCategory ac);
    void forgetFrame(int64_t packetId);
    // Synchronous replacement must register its result before it returns to the event loop.
    void prepareFrameReplacement(const std::vector<int64_t>& sourceIds);
    void registerFragments(const std::vector<Packet *>& fragments, simtime_t deadline);
    // An untracked frame has no QoS DATA deadline.
    simtime_t getDeadline(const Packet *packet) const;
    bool isExpired(const Packet *packet, simtime_t time = simTime()) const;
    simtime_t getEarliestDeadline(const std::set<int64_t>& referencedFrames) const;
    std::vector<int64_t> getExpiredFrameIds(const std::set<int64_t>& referencedFrames) const;
    // The queue owner retains this borrowed packet until it removes its lifetime entry.
    Packet *findFrame(int64_t packetId) const;
    size_t getNumFrames() const { return frames.size(); }
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif

