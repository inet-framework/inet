// SPDX-License-Identifier: LGPL-3.0-or-later
#include "inet/transportlayer/dtls/DtlsPostHandshakeExchange.h"

namespace inet::dtls {

DtlsPostHandshakeExchange::DtlsPostHandshakeExchange(cSimpleModule& owner, const std::string& name) : owner(owner)
{
    retransmissionTimer = std::make_unique<cMessage>(("dtls" + name + "Retransmission").c_str());
    deadline = std::make_unique<cMessage>(("dtls" + name + "Deadline").c_str());
}

DtlsPostHandshakeExchange::~DtlsPostHandshakeExchange()
{
    cancelTimers();
}

void DtlsPostHandshakeExchange::cancelTimers()
{
    owner.cancelEvent(retransmissionTimer.get());
    owner.cancelEvent(deadline.get());
}

} // namespace inet::dtls
