//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_ONETIDBLOCKACKREQVARIANT_H
#define __INET_ONETIDBLOCKACKREQVARIANT_H

#include <optional>

#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"

namespace inet {
namespace ieee80211 {

enum class OneTidBlockAckReqVariant
{
    BASIC,
    COMPRESSED,
};

struct OneTidBlockAckReqDetails
{
    Ptr<const Ieee80211BlockAckReq> blockAckReq;
    OneTidBlockAckReqVariant variant;
    Tid tid;
    SequenceNumberCyclic startingSequenceNumber;
};

inline std::optional<OneTidBlockAckReqDetails> getOneTidBlockAckReqDetails(const Ptr<const Ieee80211MacHeader>& header)
{
    if (auto basicBlockAckReq = dynamicPtrCast<const Ieee80211BasicBlockAckReq>(header))
        return OneTidBlockAckReqDetails { basicBlockAckReq, OneTidBlockAckReqVariant::BASIC,
            static_cast<Tid>(basicBlockAckReq->getTidInfo()), basicBlockAckReq->getStartingSequenceNumber() };
    else if (auto compressedBlockAckReq = dynamicPtrCast<const Ieee80211CompressedBlockAckReq>(header))
        return OneTidBlockAckReqDetails { compressedBlockAckReq, OneTidBlockAckReqVariant::COMPRESSED,
            static_cast<Tid>(compressedBlockAckReq->getTidInfo()), compressedBlockAckReq->getStartingSequenceNumber() };
    else
        return std::nullopt;
}

// IEEE Std 802.11-2024, 9.3.1.7, 9.3.1.8, and 10.25.5: correlate the BAR RA
// with the BA TA, TID, and selected BlockAck variant before accepting a response.
inline bool isMatchingOneTidBlockAckResponse(const OneTidBlockAckReqDetails& blockAckReqDetails, const Ptr<const Ieee80211BlockAck>& blockAck)
{
    if (blockAckReqDetails.blockAckReq->getReceiverAddress() != blockAck->getTransmitterAddress())
        return false;
    if (blockAckReqDetails.variant == OneTidBlockAckReqVariant::BASIC) {
        auto basicBlockAck = dynamicPtrCast<const Ieee80211BasicBlockAck>(blockAck);
        return basicBlockAck != nullptr && basicBlockAck->getTidInfo() == blockAckReqDetails.tid;
    }
    else {
        auto compressedBlockAck = dynamicPtrCast<const Ieee80211CompressedBlockAck>(blockAck);
        return compressedBlockAck != nullptr && compressedBlockAck->getTidInfo() == blockAckReqDetails.tid;
    }
}

} // namespace ieee80211
} // namespace inet

#endif
