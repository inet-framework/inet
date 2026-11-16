// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef INET_DTLSASSOCIATION_H
#define INET_DTLSASSOCIATION_H

#include <array>
#include <deque>
#include <memory>
#include <set>
#include <vector>

#include "inet/transportlayer/contract/dtls/IDtls.h"
#include "inet/transportlayer/dtls/DtlsRecordState.h"
#include "inet/transportlayer/tls/TlsHandshakeParameters.h"

namespace inet::dtls {

class Dtls;
class DtlsClientHello;

/** Per-peer handshake, record and application state owned by one DTLS engine. */
class INET_API DtlsAssociation
{
    friend class Dtls;

  public:
    using AssociationId = IDtls::AssociationId;
    enum class HandshakeState { CLIENT_WAIT_SERVER_HELLO, CLIENT_WAIT_SERVER_EXTENSIONS, CLIENT_WAIT_SERVER_CERTIFICATE,
        CLIENT_WAIT_SERVER_CERTIFICATE_VERIFY, CLIENT_WAIT_SERVER_FINISHED, CLIENT_WAIT_FINISHED_HANDOFF,
        SERVER_WAIT_CLIENT_CERTIFICATE, SERVER_WAIT_CLIENT_CERTIFICATE_VERIFY, SERVER_WAIT_CLIENT_FINISHED, ESTABLISHED };

  protected:
    static constexpr std::array<uint8_t, 32> HELLO_RETRY_RANDOM = {
        0xcf, 0x21, 0xad, 0x74, 0xe5, 0x9a, 0x61, 0x11, 0xbe, 0x1d, 0x8c, 0x02, 0x1e, 0x65, 0xb8, 0x91,
        0xc2, 0xa2, 0x11, 0x16, 0x7a, 0xbb, 0x8c, 0x5e, 0x07, 0x9e, 0x09, 0xe2, 0xc8, 0xa8, 0x33, 0x9c
    };
    Dtls& engine;
    AssociationId id = 0;
    AssociationId replacementId = 0;
    IDtls::ICallback *callback = nullptr;
    L3Address peerAddress;
    int peerPort = -1;
    L3Address localAddress;
    int localPort = -1;
    HandshakeState state = HandshakeState::CLIENT_WAIT_SERVER_HELLO;
    tls::TlsHandshakeParameters::Offer offer;
    std::unique_ptr<DtlsRecordState> receiveEpoch2;
    std::unique_ptr<DtlsRecordState> receiveEpoch3;
    uint64_t nextEpoch2Record = 0;
    uint64_t nextEpoch3Record = 0;
    size_t receivedBytes = 0;
    size_t sentBytes = 0;
    size_t reservedBytes = 0;
    bool serverFlightQueued = false;
    bool certificateProfile = false;
    bool clientAuthRequested = false;
    bool ownFinishedHandedOff = false;
    bool peerFinishedProcessed = false;
    uint64_t peerFinishedRecord = 0;
    bool addressValidated = false;
    bool retried = false;
    bool retryRequestedShare = false;
    std::vector<uint8_t> retryCookie;
    std::array<uint8_t, 32> clientRandom{};
    std::array<uint8_t, 32> clientShare{};
    uint16_t serverMessageBase = 0;
    uint16_t nextPeerMessage = 0;
    std::deque<std::unique_ptr<Packet>> pendingPackets;
    size_t pendingBytes = 0;
    std::set<long> processingPacketIds;
    simtime_t startedAt = SIMTIME_ZERO;
    std::unique_ptr<cMessage> handshakeDeadline;

    void processHandshake(uint64_t sequence, Packet *body);
    static int validateClientHello(const Ptr<const DtlsClientHello>& hello, bool certificateProfile, const std::string& identity,
                                   tls::TlsHandshakeParameters::Offer& offer, tls::TlsHandshakeParameters::Selection& selection);
    void processServerHello(uint16_t messageSequence, Packet *body, const L3Address& localAddress, int localPort);
    void sendServerFlight();
    void sendClientHello(const std::vector<uint8_t>& cookie);
    void serverFinishedHandedOff();

  public:
    explicit DtlsAssociation(Dtls& engine) : engine(engine) {}
};

} // namespace inet::dtls
#endif
