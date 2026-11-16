//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef INET_IDTLS_H
#define INET_IDTLS_H

#include "inet/common/packet/Packet.h"
#include "inet/networklayer/common/L3Address.h"

namespace inet::dtls {

/** The local interface of a datagram security engine. */
class INET_API IDtls
{
  public:
    using AssociationId = uint64_t;

    class INET_API ICallback
    {
      public:
        virtual ~ICallback() = default;
        virtual void accepted(AssociationId id, const L3Address& localAddress, int localPort, const L3Address& peerAddress, int peerPort) = 0;
        virtual void established(AssociationId id) = 0;
        // Packet ownership is transferred to the callback.
        virtual void dataArrived(AssociationId id, Packet *packet) = 0;
        // A rejected or cancelled application packet; the association may remain usable.
        virtual void sendError(AssociationId id, long packetId, const char *reason) = 0;
        // Invalidates the named handle. A forced local close may subsequently
        // call closed(id); the borrowed callback must remain alive for both.
        virtual void error(AssociationId id, const char *reason) = 0;
        virtual void closed(AssociationId id) = 0;
    };

    virtual ~IDtls() = default;
    // Registration is ignored while the engine is not operating.
    virtual void listen(ICallback *callback) = 0;
    // Returns zero when admission fails synchronously; valid handles are nonzero.
    virtual AssociationId open(const L3Address& peerAddress, int peerPort, ICallback *callback) = 0;
    // Ownership is transferred to the engine, including on rejection.
    virtual void send(AssociationId id, Packet *packet, ICallback *requester) = 0;
    virtual void close(AssociationId id) = 0;
    // Returns false for a synchronous rejection, without failing the association.
    virtual bool requestKeyUpdate(AssociationId id, bool requestPeerUpdate) = 0;
};

} // namespace inet::dtls

#endif
