//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_TXOPFS_H
#define __INET_TXOPFS_H

#include "inet/linklayer/ieee80211/mac/framesequence/GenericFrameSequences.h"
#include "inet/linklayer/ieee80211/mac/originator/OriginatorQosAckPolicy.h"

namespace inet {
namespace ieee80211 {

class INET_API TxOpFs : public AlternativesFs
{

  public:
    TxOpFs();
    virtual ~TxOpFs() {}

    virtual int selectTxOpSequence(AlternativesFs *frameSequence, FrameSequenceContext *context);
    virtual bool isRtsCtsNeeded(OptionalFs *frameSequence, FrameSequenceContext *context);
    virtual bool isBlockAckReqRtsCtsNeeded(OptionalFs *frameSequence, FrameSequenceContext *context);
    virtual int selectMgmtOrDataQap(AlternativesFs *frameSequence, FrameSequenceContext *context);
};

} // namespace ieee80211
} // namespace inet

#endif

