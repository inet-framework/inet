//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_TXOPFS_H
#define __INET_TXOPFS_H
#include "inet/linklayer/ieee80211/mac/framesequence/FrameSequenceContext.h"
namespace inet {
namespace ieee80211 {
class INET_API TxOpFs : public IFrameSequence
{
  protected:
    size_t step = 0;
  public:
    virtual void startSequence(FrameSequenceContext *context, int firstStep) override;
    virtual IFrameSequenceStep *prepareStep(FrameSequenceContext *context) override;
    virtual bool completeStep(FrameSequenceContext *context) override;
    virtual std::string getHistory() const override;
};
} // namespace ieee80211
} // namespace inet
#endif
