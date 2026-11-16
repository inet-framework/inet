//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_PROTOCOLTEST_COAPDTLS_COAPDTLSTESTPEER_H
#define __INET_PROTOCOLTEST_COAPDTLS_COAPDTLSTESTPEER_H

#include <cstdint>
#include <functional>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "inet/applications/base/ApplicationBase.h"
#include "inet/common/packet/Packet.h"
#include "inet/networklayer/common/L3Address.h"
#include "inet/transportlayer/contract/dtls/DtlsSocket.h"

namespace inet {
namespace protocoltest {

class INET_API CoapDtlsTestPeer : public ApplicationBase, public dtls::IDtls::ICallback
{
  public:
    enum ControlState {
        ESTABLISHED = 1,
        CLOSED = 2,
        ERROR = -1,
    };

    struct ReceivedRecord
    {
        simtime_t arrivalTime = SIMTIME_ZERO;
        uint64_t sessionId = 0;
        std::vector<uint8_t> bytes;
    };

    using InboundCallback = std::function<void(CoapDtlsTestPeer&, const ReceivedRecord&)>;

  protected:
    using cSimpleModule::error;
    static simsignal_t controlStateChangedSignal;

    dtls::DtlsSocket dtlsSocket;
    bool dtlsBound = false;
    bool serverMode = false;

    std::set<uint64_t> activeSessions;
    std::vector<uint64_t> acceptedIds;
    std::vector<uint64_t> establishedIds;
    std::vector<uint64_t> closedIds;
    std::vector<std::pair<uint64_t, std::string>> errors;
    std::vector<ReceivedRecord> receivedRecords;
    InboundCallback inboundCallback;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessageWhenUp(cMessage *msg) override;
    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;

    virtual void accepted(dtls::IDtls::AssociationId id, const L3Address& localAddress, int localPort,
            const L3Address& peerAddress, int peerPort) override;
    virtual void established(dtls::IDtls::AssociationId id) override;
    virtual void dataArrived(dtls::IDtls::AssociationId id, Packet *packet) override;
    virtual void sendError(dtls::IDtls::AssociationId id, long packetId, const char *reason) override;
    virtual void error(dtls::IDtls::AssociationId id, const char *reason) override;
    virtual void closed(dtls::IDtls::AssociationId id) override;

  public:
    CoapDtlsTestPeer() = default;
    virtual ~CoapDtlsTestPeer() = default;

    uint64_t openSession(const L3Address& destAddr, int destPort = 5684);
    uint64_t openSession(const std::string& destAddrStr, int destPort = 5684);
    void sendBytes(uint64_t sessionId, const std::vector<uint8_t>& bytes, const char *packetName = "CoapDtlsRawMsg");
    void closeSession(uint64_t sessionId);
    bool requestKeyUpdate(uint64_t sessionId, bool requestPeerUpdate = false);

    bool isSessionEstablished(uint64_t sessionId) const;
    bool isSessionClosed(uint64_t sessionId) const;
    const std::vector<uint64_t>& getAcceptedIds() const { return acceptedIds; }
    const std::vector<uint64_t>& getEstablishedIds() const { return establishedIds; }
    const std::vector<uint64_t>& getClosedIds() const { return closedIds; }
    const std::vector<std::pair<uint64_t, std::string>>& getErrors() const { return errors; }
    const std::vector<ReceivedRecord>& getReceived() const { return receivedRecords; }
    void clearReceived() { receivedRecords.clear(); }
    void setInboundCallback(InboundCallback cb) { inboundCallback = std::move(cb); }
};

} // namespace protocoltest
} // namespace inet

#endif
