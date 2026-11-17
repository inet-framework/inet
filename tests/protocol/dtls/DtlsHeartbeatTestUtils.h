// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef INET_PROTOCOLTEST_DTLSHEARTBEATTESTUTILS_H
#define INET_PROTOCOLTEST_DTLSHEARTBEATTESTUTILS_H

#include "DtlsTestUtils.h"
#include "inet/transportlayer/dtls/Dtls.h"
#include "inet/common/socket/SocketTag_m.h"
#include "inet/transportlayer/contract/udp/UdpCommand_m.h"

namespace inet::protocoltest::dtls {

// Exposes the internal production probe primitive for this stage, before its
// migration and PMTU consumers exist. No handshake, probe or recovery substitute.
class ProbeDtls : public inet::dtls::Dtls
{
  public:
    std::vector<bool> probeAdmissions;
    int socketIdForTest() const { return udpSocket.getSocketId(); }
    void setNextConnectionIdForTest(uint64_t value) { nextConnectionId = value; }
    bool hasNoAdmissionStateForTest() const { return associations.empty() && prevalidationAttempts.empty() && cidIndex.empty(); }
    void injectStaleSocketClose(int socketId) {
        Enter_Method("injectStaleSocketClose");
        auto indication = new Indication("old UDP socket indication", UDP_I_SOCKET_CLOSED);
        indication->addTag<SocketInd>()->setSocketId(socketId);
        handleMessageWhenUp(indication);
    }
    void probePeer(size_t bytes = 0) {
        auto handle = associations.empty() ? 0 : associations.begin()->first;
        probeAdmissions.push_back(startHeartbeatProbe(handle, bytes));
    }
};
Define_Module(ProbeDtls);

inline bool heartbeat(const MatchContext& context, int type)
{
    auto body = findChunk<inet::dtls::DtlsHeartbeat>(context.event.packet);
    return body && body->getType() == type;
}

inline bool packetHeartbeat(const Packet *packet, int type)
{
    auto body = findChunk<inet::dtls::DtlsHeartbeat>(packet);
    return body && body->getType() == type;
}

} // namespace inet::protocoltest::dtls
#endif
