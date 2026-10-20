// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef __INET_IIEEE80211MAC_H
#define __INET_IIEEE80211MAC_H

#include "inet/linklayer/common/MacAddress.h"
#include "inet/linklayer/contract/IMacProtocol.h"

namespace inet {
namespace ieee80211 {

/**
 * C++ contract for the IIeee80211Mac NED module interface.
 */
class INET_API IIeee80211Mac : public IMacProtocol
{
  public:
    virtual ~IIeee80211Mac() {}

    /**
     * Resets MAC state that belongs to the peer's old association before replacement or loss.
     * Management supplies the old peer address before it changes the association state.
     * The call completes local cleanup without a DELBA exchange.
     * The current implementation clears both Block Ack roles, pending agreement procedures,
     * agreement actions, queued QoS DATA, and recipient reorder, duplicate, and reassembly state.
     * The reset preserves other peers and the context of accepted physical exchanges.
     * Referenced frames remain valid until the active frame sequence releases them.
     * The call does not change management or MIB association state.
     * Implementations without Block Ack support currently perform no cleanup.
     */
    virtual void resetPeerAssociationContext(const MacAddress& peer) = 0;
};

} // namespace ieee80211
} // namespace inet

#endif
