// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef __INET_IBLOCKACKAGREEMENTLIFECYCLE_H
#define __INET_IBLOCKACKAGREEMENTLIFECYCLE_H

#include "inet/linklayer/common/MacAddress.h"

namespace inet {
namespace ieee80211 {

// Management supplies the old peer identity before association replacement or loss.
class INET_API IBlockAckAgreementLifecycle
{
  public:
    virtual ~IBlockAckAgreementLifecycle() {}
    virtual void resetPeerAgreements(const MacAddress& peer) = 0;
};

} // namespace ieee80211
} // namespace inet

#endif
