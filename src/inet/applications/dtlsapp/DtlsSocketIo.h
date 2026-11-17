// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef INET_DTLSSOCKETIO_H
#define INET_DTLSSOCKETIO_H

#include "inet/applications/base/ApplicationBase.h"
#include "inet/common/scenario/IScriptable.h"
#include "inet/transportlayer/contract/dtls/DtlsSocket.h"

namespace inet::dtls {

class INET_API DtlsSocketIo : public ApplicationBase, public IDtls::ICallback, public IScriptable
{
  protected:
    DtlsSocket socket;
    IDtls::AssociationId clientId = 0;
    bool server = false;

    void handleMessageWhenUp(cMessage *message) override;
    void handleStartOperation(LifecycleOperation *operation) override;
    void handleStopOperation(LifecycleOperation *operation) override;
    void handleCrashOperation(LifecycleOperation *operation) override;
    void accepted(IDtls::AssociationId id, const L3Address& localAddress, int localPort, const L3Address& peerAddress, int peerPort) override;
    void established(IDtls::AssociationId id) override;
    void dataArrived(IDtls::AssociationId id, Packet *packet) override;
    void earlyDataArrived(IDtls::AssociationId id, Packet *packet) override;
    void earlyDataStatus(IDtls::AssociationId id, bool accepted) override;
    void pathChanged(IDtls::AssociationId id, const DtlsPath& path) override;
    void authenticationResult(IDtls::AssociationId id, bool success, const char *identity) override;
    void maximumDatagramSizeChanged(IDtls::AssociationId id, size_t bytes) override;
    void sendError(IDtls::AssociationId id, long packetId, const char *reason) override;
    void error(IDtls::AssociationId id, const char *reason) override;
    void closed(IDtls::AssociationId id) override;

  public:
    void processCommand(const cXMLElement& node) override;
};

} // namespace inet::dtls
#endif
