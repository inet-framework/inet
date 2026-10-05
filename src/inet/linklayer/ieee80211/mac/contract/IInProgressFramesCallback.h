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
    virtual void frameWillBeRemoved(InProgressFrames *owner, const Packet *frame) = 0;
};

} // namespace ieee80211
} // namespace inet

#endif
