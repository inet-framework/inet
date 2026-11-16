// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef INET_DTLS_H
#define INET_DTLS_H

#include <map>
#include <memory>

#include "inet/common/Protocol.h"
#include "inet/common/lifecycle/OperationalBase.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/transportlayer/contract/dtls/IDtls.h"
#include "inet/transportlayer/contract/udp/UdpSocket.h"
#include "inet/transportlayer/dtls/DtlsProcessingQueue.h"
#include "inet/transportlayer/dtls/DtlsAssociation.h"

namespace inet::dtls {

extern INET_API const Protocol dtlsProtocol;

/** Datagram security engine. All protection is modeled; no cryptography is performed. */
class INET_API Dtls : public OperationalBase, public UdpSocket::ICallback, public IDtls
{
    friend class DtlsAssociation;
  protected:
    using ICallback = IDtls::ICallback;
    enum DropReason { MALFORMED_RECORD = 1, UNKNOWN_EPOCH = 2, REPLAY = 3, RESOURCE_LIMIT = 4 };
    using Association = DtlsAssociation;
    using HandshakeState = DtlsAssociation::HandshakeState;

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
    static simsignal_t associationEstablishedSignal;
    static simsignal_t associationFailedSignal;
    static simsignal_t establishmentTimeSignal;
    static simsignal_t recordOverheadSignal;
    static simsignal_t handshakeRetransmissionSignal;
    static simsignal_t processingDelaySignal;

  protected:
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
    void sendClientHello(AssociationId id, const std::vector<uint8_t>& cookie = {});
    void sendRetry(const L3Address& localAddress, const L3Address& peerAddress, int peerPort, const std::vector<uint8_t>& cookie, bool requestKeyShare, size_t receivedBytes);
    bool handleHandshakeDeadline(cMessage *message);
    void handlePlainHandshake(Packet *body, const L3Address& localAddress, int localPort, const L3Address& peerAddress, int peerPort, size_t datagramBytes, const std::shared_ptr<bool>& creditCounted);
    void handleProtectedHandshake(AssociationId id, uint16_t epoch, uint16_t truncatedSequence, unsigned sequenceBits, Packet *body, size_t recordBytes);
    void handleAvailableProtectedHandshake(AssociationId id, uint16_t epoch, uint16_t truncatedSequence, unsigned sequenceBits, Packet *body, size_t recordBytes);
    void processProtectedHandshake(AssociationId id, uint64_t sequence, Packet *body);
    void sendServerFlight(AssociationId id);
    Ptr<Chunk> makeCertificate() const;
    Ptr<Chunk> makeCertificateVerify() const;
    int checkCertificate(Packet *body) const;
    int checkCertificateVerify(Packet *body) const;
    void sendFinalAck(AssociationId id, uint64_t sequence);
    void sendApplication(AssociationId id, Packet *packet);
    void sendPlainHandshake(AssociationId id, uint8_t messageType, uint16_t messageSequence, const Ptr<Chunk>& body);
    void sendProtectedHandshake(AssociationId id, uint8_t messageType, uint16_t messageSequence, const Ptr<Chunk>& body, bool establishAfterSend = false, bool serverFinished = false);
    void establish(AssociationId id);
    void failAssociation(AssociationId id, const char *reason, int alertDescription = -1);
    void sendAlert(const L3Address& localAddress, const L3Address& peerAddress, int peerPort, uint8_t description, bool encrypted, uint64_t sequence = 0);
    bool enqueueProcessing(DtlsProcessingQueue *queue, AssociationId id, size_t bytes, simtime_t delay, DtlsProcessingQueue::Completion completion);
    bool containsAssociation(AssociationId id) const { return associations.find(id) != associations.end(); }
    static bool hasConsistentEncoding(const Packet *packet);

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
