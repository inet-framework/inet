// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef INET_STAGEDFRAMEVIEW_H
#define INET_STAGEDFRAMEVIEW_H

#include "inet/common/packet/Packet.h"
#include "inet/linklayer/ieee80211/mac/common/AckFrameState.h"
#include "inet/linklayer/ieee80211/mac/common/SequenceControlField.h"

namespace inet::ieee80211 {

class InProgressFrames;

struct INET_API StagedFrameView
{
    const Packet *frame = nullptr;
    const InProgressFrames *owner = nullptr;
    int64_t identity = -1;
    uint64_t epoch = 0;
    int64_t unitIdentity = -1;
    MacAddress receiver;
    int tid = -1;
    SequenceControlField sequenceControl = SequenceControlField(0, 0);
    b length = b(0);
    b originalLength = b(0);
    int fragmentCount = 0;
    int transmissions = 0;
    bool earlierFragmentRetransmitted = false;
    AckFrameState ack;
};

} // namespace inet::ieee80211

#endif
