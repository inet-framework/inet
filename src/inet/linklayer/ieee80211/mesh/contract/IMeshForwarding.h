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
    // Borrows a committed local source unit; retries/transit remain unchanged.
    virtual void finalizeSourceFrame(Packet *packet) = 0;
    virtual bool isSourceActive() const = 0;
    virtual void pathAvailable(MacAddress destination) = 0;
    virtual void discoveryFailed(MacAddress destination) = 0;
    // Expires due source packets before reporting discovery eligibility.
    virtual bool hasPendingSource(MacAddress destination) = 0;
};

} // namespace ieee80211
} // namespace inet
#endif
