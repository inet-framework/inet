// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef __INET_TXOPEXCHANGEPLANNER_H
#define __INET_TXOPEXCHANGEPLANNER_H

#include "inet/linklayer/ieee80211/mac/framesequence/FrameSequenceContext.h"

namespace inet {
namespace ieee80211 {

class INET_API TxopExchangePlanner
{
  public:
    std::unique_ptr<TxopExchangePlan> prepare(FrameSequenceContext *context,
            const TxopExchangePlan *projectedCurrent, bool forceBlockAck, bool useFastestMode = false) const;
    void prepareContinuation(FrameSequenceContext *context) const;
};

} // namespace ieee80211
} // namespace inet

#endif
