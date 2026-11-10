// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef __INET_IMESHFORWARDING_H
#define __INET_IMESHFORWARDING_H

#include "inet/linklayer/ieee80211/mac/contract/IDs.h"

namespace inet {
namespace ieee80211 {

class INET_API IMeshForwarding : public IDs
{
  public:
    // Takes ownership of an LLC MSDU before MAC encapsulation.
    virtual void processSourceMsdu(Packet *packet) = 0;
};

} // namespace ieee80211
} // namespace inet
#endif
