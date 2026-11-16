// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef INET_DTLS_H
#define INET_DTLS_H

#include <map>
#include <memory>

#include "inet/common/Protocol.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/lifecycle/OperationalBase.h"
#include "inet/transportlayer/contract/dtls/IDtls.h"
#include "inet/transportlayer/contract/udp/UdpSocket.h"
#include "inet/transportlayer/dtls/DtlsProcessingQueue.h"

namespace inet::dtls {

extern INET_API const Protocol dtlsProtocol;

class INET_API Dtls : public OperationalBase, public UdpSocket::ICallback, public IDtls
{
  protected:
    using ICallback = IDtls::ICallback;
    enum DropReason { MALFORMED_RECORD = 1, UNKNOWN_EPOCH = 2, REPLAY = 3, RESOURCE_LIMIT = 4 };
    struct Association {
        AssociationId id = 0;
        ICallback *callback = nullptr;
        L3Address peerAddress;
        int peerPort = -1;
    };
    UdpSocket udpSocket;
    ICallback *listener = nullptr;
    std::map<AssociationId, std::unique_ptr<Association>> associations;
    AssociationId nextAssociationId = 1;
    std::unique_ptr<DtlsProcessingQueue> inputProcessing;
    std::unique_ptr<DtlsProcessingQueue> outputProcessing;
    static simsignal_t packetSentToLowerSignal;
    static simsignal_t packetReceivedFromLowerSignal;
    static simsignal_t packetSentToUpperSignal;
    static simsignal_t recordAcceptedSignal;
    static simsignal_t recordDroppedSignal;

    bool isInitializeStage(int stage) const override { return stage == INITSTAGE_APPLICATION_LAYER; }
    bool isModuleStartStage(int stage) const override { return stage == ModuleStartOperation::STAGE_APPLICATION_LAYER; }
    bool isModuleStopStage(int stage) const override { return stage == ModuleStopOperation::STAGE_APPLICATION_LAYER; }
    void handleMessageWhenUp(cMessage *message) override;
    void handleStartOperation(LifecycleOperation *operation) override;
    void handleStopOperation(LifecycleOperation *operation) override;
    void handleCrashOperation(LifecycleOperation *operation) override;
    void socketDataArrived(UdpSocket *socket, Packet *packet) override;
    void socketErrorArrived(UdpSocket *socket, Indication *indication) override;
    void socketClosed(UdpSocket *socket) override;
    void discardAll(const char *reason, bool notify);
    void parseDatagram(Packet *packet);

  public:
    ~Dtls() override;
    void listen(ICallback *callback) override;
    AssociationId open(const L3Address& peerAddress, int peerPort, ICallback *callback) override;
    void send(AssociationId id, Packet *packet, ICallback *requester) override;
    void close(AssociationId id) override;
    bool requestKeyUpdate(AssociationId id, bool requestPeerUpdate) override;
};

} // namespace inet::dtls
#endif
