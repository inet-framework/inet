// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef INET_PROTOCOLTEST_DTLSHEARTBEATTESTUTILS_H
#define INET_PROTOCOLTEST_DTLSHEARTBEATTESTUTILS_H

#include "DtlsTestUtils.h"
#include "inet/transportlayer/dtls/Dtls.h"

namespace inet::protocoltest::dtls {

// Exposes the internal production probe primitive for this stage, before its
// migration and PMTU consumers exist. No handshake, probe or recovery substitute.
class ProbeDtls : public inet::dtls::Dtls
{
  public:
    std::vector<bool> probeAdmissions;
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
