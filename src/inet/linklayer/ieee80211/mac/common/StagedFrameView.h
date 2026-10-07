// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef INET_STAGEDFRAMEVIEW_H
#define INET_STAGEDFRAMEVIEW_H

#include "inet/common/packet/Packet.h"
#include "inet/linklayer/ieee80211/mac/common/AckFrameState.h"
#include "inet/linklayer/ieee80211/mac/common/SequenceControlField.h"

namespace inet::ieee80211 {

class InProgressFrames;

// This snapshot borrows its frame and owner. A copy retains neither object. FrameSequenceContext retains frame references separately and must end before the store dies. Frame removal or a lifecycle reset invalidates the snapshot. Length, receiver, traffic identifier, or sequence-control changes also invalidate the snapshot. Check InProgressFrames::isRetained() while the owner is alive before use.
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
    b length = b(0); // Complete frame length, in bits, at inspection.
    b originalLength = b(0); // Complete frame length, in bits, at extraction.
    int fragmentCount = 0;
    int transmissions = 0;
    bool earlierFragmentRetransmitted = false;
    AckFrameState ack;
};

} // namespace inet::ieee80211

#endif
