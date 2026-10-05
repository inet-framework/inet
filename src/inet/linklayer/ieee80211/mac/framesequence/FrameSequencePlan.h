// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef INET_FRAMESEQUENCEPLAN_H
#define INET_FRAMESEQUENCEPLAN_H

#include "inet/linklayer/ieee80211/mac/common/StagedFrameView.h"
#include "inet/linklayer/ieee80211/mac/contract/IFrameSequence.h"
#include "inet/physicallayer/wireless/ieee80211/mode/IIeee80211Mode.h"

namespace inet::ieee80211 {

// The transmit step owns this record. The record borrows its frame and mode. The frame store owns staged frames; an RTS transmit step owns its generated frame. The exchange context retains staged frame references for the plan's lifetime. Hypothetical: preparation records 24 Mbps while another mode query would return 6 Mbps. Execution uses the recorded 24 Mbps without another query.
struct INET_API PreparedTransmit
{
    const Packet *frame = nullptr;
    const physicallayer::IIeee80211Mode *mode = nullptr;
    b length = b(0); // Complete frame length, in bits.
    simtime_t airtime; // Physical transmission duration at the selected mode.
    simtime_t ifs; // Interframe space before this transmission.
    simtime_t duration; // Reservation after this frame, for the Duration/ID header field.
    AckPolicy ackPolicy = NORMAL_ACK;
    int offset = 0; // Step position relative to the prepared exchange.
};

// The receive step owns this record. The record borrows the request frame and response mode. The frame store or the generated transmit step owns the request frame.
struct INET_API PreparedReceive
{
    const Packet *request = nullptr;
    const physicallayer::IIeee80211Mode *mode = nullptr;
    b length = b(0); // Complete predicted response length, in bits.
    simtime_t airtime; // Physical response duration at the selected mode.
    simtime_t ifs; // Interframe space before the response.
    int offset = 0; // Step position relative to the prepared exchange.
};

// Each node records one use of one object in the current constructor tree. The node owns its child plans and prepared steps. It borrows the sequence object and the candidate's frame and owner. The exchange context owns the root plan and retains staged frame references. These records do not retain the sequence tree, frame store, or mode set.
struct INET_API FrameSequencePlan
{
    const IFrameSequence *sequence = nullptr;
    int offset = 0;
    int stepCount = 0;
    int selectedChild = -1;
    bool continuation = false;
    bool overrunAllowed = false;
    simtime_t duration; // Complete exchange cost: each frame's airtime plus each required interframe space.
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
