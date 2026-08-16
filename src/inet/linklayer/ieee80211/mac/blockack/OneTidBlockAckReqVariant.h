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

} // namespace ieee80211
} // namespace inet

#endif
