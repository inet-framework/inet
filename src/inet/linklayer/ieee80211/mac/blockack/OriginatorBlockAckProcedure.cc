//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/blockack/OriginatorBlockAckProcedure.h"

namespace inet {
namespace ieee80211 {

const Ptr<Ieee80211BlockAckReq> OriginatorBlockAckProcedure::buildCompressedBlockAckReqFrame(const MacAddress& receiverAddress, Tid tid, SequenceNumberCyclic startingSequenceNumber) const
{
    throw cRuntimeError("Unsupported feature");
    auto blockAckReq = makeShared<Ieee80211CompressedBlockAckReq>();
    blockAckReq->setReceiverAddress(receiverAddress);
    blockAckReq->setStartingSequenceNumber(startingSequenceNumber);
    blockAckReq->setTidInfo(tid);
    return blockAckReq;
}

const Ptr<Ieee80211BlockAckReq> OriginatorBlockAckProcedure::buildBasicBlockAckReqFrame(const MacAddress& receiverAddress, Tid tid, SequenceNumberCyclic startingSequenceNumber) const
{
    auto blockAckReq = makeShared<Ieee80211BasicBlockAckReq>();
    blockAckReq->setReceiverAddress(receiverAddress);
    blockAckReq->setStartingSequenceNumber(startingSequenceNumber);
    blockAckReq->setTidInfo(tid);
    return blockAckReq;
}

} /* namespace ieee80211 */
} /* namespace inet */

