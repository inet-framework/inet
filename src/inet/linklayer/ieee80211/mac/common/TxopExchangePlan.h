//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_TXOPEXCHANGEPLAN_H
#define __INET_TXOPEXCHANGEPLAN_H

#include "inet/linklayer/ieee80211/mac/contract/IFrameSequence.h"

namespace inet {
namespace ieee80211 {

struct INET_API TxopFrameIdentity
{
    MacAddress receiver;
    int tid = -1;
    int sequenceNumber = -1;

    bool operator==(const TxopFrameIdentity& other) const {
        return receiver == other.receiver && tid == other.tid && sequenceNumber == other.sequenceNumber;
    }
    bool operator<(const TxopFrameIdentity& other) const {
        return std::tie(receiver, tid, sequenceNumber) < std::tie(other.receiver, other.tid, other.sequenceNumber);
    }
};

enum class TxopAdmissionReason {
    WITHIN_LIMIT = 0, ZERO_LIMIT_FRAME = 1, UNCHANGED_RETRY = 2, BLOCK_ACK_MSDU = 3,
    CONTROL = 4, RETRIED_FRAGMENT_FAMILY = 5, SIXTEEN_FRAGMENTS = 6, GROUP = 7,
    LIMIT_EXCEEDED = 8, DIFFERENT_ZERO_LIMIT_FRAME = 9, RESERVATION_EXPIRED = 10, NO_PRIOR_CONTROL_MODE = 11, INACTIVE = 12
};

struct INET_API TxopAdmissionDecision
{
    bool accepted = false;
    TxopAdmissionReason reason = TxopAdmissionReason::INACTIVE;
    simtime_t start;
    simtime_t end;
    simtime_t remaining;
};

// The context owns every step. This value borrows those same execution objects.
struct INET_API TxopExchangePlan
{
    std::vector<IFrameSequenceStep *> steps;
    Packet *payload = nullptr;
    TxopFrameIdentity identity;
    int fragmentNumber = -1;
    int64_t packetId = -1;
    AckPolicy ackPolicy = AckPolicy::NORMAL_ACK;
    uint64_t rateRevision = 0;
    bool dataOrManagement = false;
    bool group = false;
    bool retry = false;
    bool unchangedRetry = false;
    bool initialBlockAckMsdu = false;
    bool initialFragmentAfterRetry = false;
    bool sixteenFragments = false;
    bool missingPriorControlMode = false;
    TxopAdmissionDecision admission;

    simtime_t getDuration() const {
        simtime_t duration;
        for (auto step : steps)
            duration += step->getPreparedInterval() + step->getPpduDuration();
        return duration;
    }
    simtime_t getLeadingInterval() const { return steps.empty() ? SIMTIME_ZERO : steps.front()->getPreparedInterval(); }
};

} // namespace ieee80211
} // namespace inet

#endif
