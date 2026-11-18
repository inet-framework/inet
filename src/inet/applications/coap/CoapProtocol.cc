//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/applications/coap/CoapProtocol.h"

#include "inet/common/ProtocolGroup.h"

namespace inet {
namespace coap {

const Protocol& getCoapProtocol()
{
    static const Protocol coap("coap", "CoAP");
    return coap;
}

namespace {

void registerCoapProtocol()
{
    const Protocol& coap = getCoapProtocol();
    if (ProtocolGroup::getUdpProtocolGroup()->findProtocol(COAP_UDP_PORT) == nullptr)
        ProtocolGroup::getUdpProtocolGroup()->addProtocol(COAP_UDP_PORT, &coap);
}

} // namespace

EXECUTE_PRE_NETWORK_SETUP(registerCoapProtocol());

} // namespace coap
} // namespace inet
