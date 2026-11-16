// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef INET_DTLSPOSTHANDSHAKEEXCHANGE_H
#define INET_DTLSPOSTHANDSHAKEEXCHANGE_H

#include <memory>
#include <string>

#include "inet/transportlayer/dtls/DtlsHandshakeFlight.h"

namespace inet::dtls {

/** Recovery state for one independently acknowledged post-handshake exchange. */
class INET_API DtlsPostHandshakeExchange
{
  protected:
    cSimpleModule& owner;

  public:
    std::shared_ptr<DtlsHandshakeFlight> flight;
    std::unique_ptr<cMessage> retransmissionTimer;
    std::unique_ptr<cMessage> deadline;
    // RFC 9147 Section 5.8.2: initial retry interval in seconds.
    simtime_t timeout = 1;

    DtlsPostHandshakeExchange(cSimpleModule& owner, const std::string& name);
    ~DtlsPostHandshakeExchange();
    DtlsPostHandshakeExchange(const DtlsPostHandshakeExchange&) = delete;
    DtlsPostHandshakeExchange& operator=(const DtlsPostHandshakeExchange&) = delete;
    void cancelTimers();
};

} // namespace inet::dtls
#endif
