//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef INET_DTLSSOCKET_H
#define INET_DTLSSOCKET_H

#include "inet/transportlayer/contract/dtls/IDtls.h"

namespace inet::dtls {

/** A non-owning application facade for an IDtls engine and its callbacks. */
class INET_API DtlsSocket
{
  protected:
    IDtls *engine = nullptr;
    IDtls::ICallback *callback = nullptr;

  public:
    using AssociationId = IDtls::AssociationId;
    void setEngine(IDtls *value) { engine = value; }
    void setCallback(IDtls::ICallback *value) { callback = value; }
    void listen() { checkEngine(); engine->listen(callback); }
    AssociationId open(const L3Address& peerAddress, int peerPort) { checkEngine(); return engine->open(peerAddress, peerPort, callback); }
    void send(AssociationId id, Packet *packet) { checkEngine(); engine->send(id, packet, callback); }
    void close(AssociationId id) { checkEngine(); engine->close(id); }
    bool requestKeyUpdate(AssociationId id, bool requestPeerUpdate = false) { checkEngine(); return engine->requestKeyUpdate(id, requestPeerUpdate); }

  protected:
    void checkEngine() const {
        if (engine == nullptr || callback == nullptr)
            throw cRuntimeError("DtlsSocket requires an engine and callback");
    }
};

} // namespace inet::dtls

#endif
