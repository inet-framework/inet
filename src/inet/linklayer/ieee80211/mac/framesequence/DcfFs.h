//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_DCFFS_H
#define __INET_DCFFS_H

#include "inet/linklayer/ieee80211/mac/framesequence/GenericFrameSequences.h"

namespace inet {
namespace ieee80211 {

class INET_API DcfFs : public AlternativesFs
{
  public:
    DcfFs();

    virtual int selectDcfSequence(AlternativesFs *frameSequence, FrameSequenceContext *context);
    virtual int selectSelfCtsOrRtsCts(AlternativesFs *frameSequence, FrameSequenceContext *context);
    virtual int selectMulticastDataOrMgmt(AlternativesFs *frameSequence, FrameSequenceContext *context);
    virtual bool hasMoreFragments(RepeatingFs *frameSequence, FrameSequenceContext *context);
    virtual bool isSelfCtsNeeded(OptionalFs *frameSequence, FrameSequenceContext *context);
    virtual bool isRtsCtsNeeded(OptionalFs *frameSequence, FrameSequenceContext *context);
    virtual bool isCtsOrRtsCtsNeeded(OptionalFs *frameSequence, FrameSequenceContext *context);
    virtual bool isBroadcastManagementOrGroupDataSequenceNeeded(AlternativesFs *frameSequence, FrameSequenceContext *context);
    virtual bool isFragFrameSequenceNeeded(AlternativesFs *frameSequence, FrameSequenceContext *context);
};

} // namespace ieee80211
} // namespace inet

#endif

