// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef __INET_ISELECTIVEPACKETCOLLECTION_H
#define __INET_ISELECTIVEPACKETCOLLECTION_H

#include <functional>

#include "inet/common/packet/Packet.h"

namespace inet {
namespace queueing {

class INET_API UnsupportedPacketSelection : public cRuntimeError
{
  public:
    explicit UnsupportedPacketSelection(const char *message) : cRuntimeError("%s", message) {}
};

// Optional atomic selection in the configured service order.
// UnsupportedPacketSelection identifies an unavailable capability; other errors retain their original type.
class INET_API ISelectivePacketCollection
{
  public:
    virtual ~ISelectivePacketCollection() {}
    // The result is borrowed. The query changes no packet or queue state.
    virtual Packet *peekEligiblePacket(const std::function<bool(Packet *)>& predicate) const = 0;
    // The caller receives ownership. Each layer applies its ordinary pull effects once.
    virtual Packet *extractPacket(Packet *packet) = 0;
};

} // namespace queueing
} // namespace inet

#endif
