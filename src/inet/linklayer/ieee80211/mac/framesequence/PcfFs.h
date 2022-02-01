//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PCFFS_H
#define __INET_PCFFS_H

#include "inet/linklayer/ieee80211/mac/framesequence/GenericFrameSequences.h"

namespace inet {
namespace ieee80211 {

class INET_API PcfFs : public AlternativesFs
{
  public:
    PcfFs();

    virtual int selectPcfSequence(AlternativesFs *frameSequence, FrameSequenceContext *context);
};

} // namespace ieee80211
} // namespace inet

#endif

