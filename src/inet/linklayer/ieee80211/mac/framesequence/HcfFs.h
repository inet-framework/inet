//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_HCFFS_H
#define __INET_HCFFS_H
#include "inet/linklayer/ieee80211/mac/framesequence/GenericFrameSequences.h"
namespace inet {
namespace ieee80211 {
class INET_API HcfFs : public RepeatingFs
{
  protected:
    bool prepareExchange(FrameSequenceContext *context);
  public:
    HcfFs();
};
} // namespace ieee80211
} // namespace inet
#endif
