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
    AssociationId open(const L3Address& peerAddress, int peerPort, bool resumeSession) { checkEngine(); return engine->open(peerAddress, peerPort, callback, resumeSession); }
    bool isResumed(AssociationId id) const { checkEngine(); return engine->isResumed(id); }
    AssociationId open(const L3Address& peerAddress, int peerPort, const DtlsOpenOptions& options) { checkEngine(); return engine->open(peerAddress, peerPort, callback, options); }
    void sendEarly(AssociationId id, Packet *packet) { checkEngine(); engine->sendEarly(id, packet, callback); }
    void send(AssociationId id, Packet *packet) { checkEngine(); engine->send(id, packet, callback); }
    void close(AssociationId id) { checkEngine(); engine->close(id); }
    bool requestKeyUpdate(AssociationId id, bool requestPeerUpdate = false) { checkEngine(); return engine->requestKeyUpdate(id, requestPeerUpdate); }
    bool requestConnectionIds(AssociationId id, int count = 1) { checkEngine(); return engine->requestConnectionIds(id, count); }
    bool provideConnectionIds(AssociationId id, int count = 1, bool immediate = true) { checkEngine(); return engine->provideConnectionIds(id, count, immediate); }
    bool activateSpareConnectionId(AssociationId id) { checkEngine(); return engine->activateSpareConnectionId(id); }
    bool migrateLocal(AssociationId id, const L3Address& address, int interfaceId = -1) { checkEngine(); return engine->migrateLocal(id, address, interfaceId); }
    bool rebindLocalPort(AssociationId id, int port) { checkEngine(); return engine->rebindLocalPort(id, port); }
    bool requestClientAuthentication(AssociationId id) { checkEngine(); return engine->requestClientAuthentication(id); }
    bool getPeerAuthentication(AssociationId id, DtlsPeerAuthentication& result) const { checkEngine(); return engine->getPeerAuthentication(id, result); }

  protected:
    void checkEngine() const {
        if (engine == nullptr || callback == nullptr)
            throw cRuntimeError("DtlsSocket requires an engine and callback");
    }
};

} // namespace inet::dtls

#endif
