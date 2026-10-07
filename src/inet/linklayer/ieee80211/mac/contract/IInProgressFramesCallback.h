// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef INET_IINPROGRESSFRAMESCALLBACK_H
#define INET_IINPROGRESSFRAMESCALLBACK_H

#include "inet/common/INETDefs.h"

namespace inet {

class Packet;

namespace ieee80211 {

class InProgressFrames;

class INET_API IInProgressFramesCallback
{
  public:
    virtual ~IInProgressFramesCallback() = default;
    // The store calls this method before it removes the frame from its active set. The callback borrows owner and frame; it must not delete either object. For example, HCF invalidates a plan that references this frame before removal.
    virtual void frameWillBeRemoved(InProgressFrames *owner, const Packet *frame) = 0;
};

} // namespace ieee80211
} // namespace inet

#endif
