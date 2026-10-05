// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef INET_ACKFRAMESTATE_H
#define INET_ACKFRAMESTATE_H

#include "inet/common/INETDefs.h"

namespace inet::ieee80211 {

// A read-only copy of the ACK owner's state. Preparation cannot establish transmission evidence.
struct INET_API AckFrameState
{
    enum class Phase {
        FRAME_NOT_YET_TRANSMITTED,
        NO_ACK_REQUIRED,
        BLOCK_ACK_NOT_YET_REQUESTED,
        WAITING_FOR_NORMAL_ACK,
        WAITING_FOR_BLOCK_ACK,
        NORMAL_ACK_NOT_ARRIVED,
        NORMAL_ACK_ARRIVED,
        BLOCK_ACK_ARRIVED_UNACKED,
        BLOCK_ACK_ARRIVED_ACKED,
        BLOCK_ACK_NOT_ARRIVED
    };

    Phase phase = Phase::FRAME_NOT_YET_TRANSMITTED;
    bool eligible = false;
    bool outstanding = false;
    bool transmitted = false;
};

} // namespace inet::ieee80211

#endif
