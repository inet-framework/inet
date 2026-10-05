// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef INET_FRAMESEQUENCEPLAN_H
#define INET_FRAMESEQUENCEPLAN_H

#include "inet/linklayer/ieee80211/mac/common/StagedFrameView.h"
#include "inet/linklayer/ieee80211/mac/contract/IFrameSequence.h"
#include "inet/physicallayer/wireless/ieee80211/mode/IIeee80211Mode.h"

namespace inet::ieee80211 {

struct INET_API PreparedTransmit
{
    const Packet *frame = nullptr;
    const physicallayer::IIeee80211Mode *mode = nullptr;
    b length = b(0);
    simtime_t airtime;
    simtime_t ifs;
    simtime_t duration;
    AckPolicy ackPolicy = NORMAL_ACK;
    int offset = 0;
};

struct INET_API PreparedReceive
{
    const Packet *request = nullptr;
    const physicallayer::IIeee80211Mode *mode = nullptr;
    b length = b(0);
    simtime_t airtime;
    simtime_t ifs;
    int offset = 0;
};

// Each node records one use of one object in the existing constructor tree.
struct INET_API FrameSequencePlan
{
    const IFrameSequence *sequence = nullptr;
    int offset = 0;
    int stepCount = 0;
    int selectedChild = -1;
    bool continuation = false;
    bool overrunAllowed = false;
    simtime_t duration;
    StagedFrameView candidate;
    std::vector<std::unique_ptr<FrameSequencePlan>> children;
    std::vector<std::unique_ptr<IFrameSequenceStep>> steps;

    void append(std::unique_ptr<FrameSequencePlan> child);
    [[nodiscard]] std::vector<IFrameSequenceStep *> flatten() const;
    [[nodiscard]] simtime_t remainingDuration(int offset, bool excludeIfs) const;
};

struct INET_API FrameSequencePlanResult
{
    enum class Status { READY, EMPTY, UNSUPPORTED };
    Status status = Status::UNSUPPORTED;
    std::unique_ptr<FrameSequencePlan> plan;

    FrameSequencePlanResult(Status status) : status(status) {}
    FrameSequencePlanResult(std::unique_ptr<FrameSequencePlan> plan) : status(Status::READY), plan(std::move(plan)) {}
};

} // namespace inet::ieee80211

#endif
