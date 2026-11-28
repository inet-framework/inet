//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_COAP_COAPPROTOCOL_H
#define __INET_COAP_COAPPROTOCOL_H

#include <cstdint>

#include "inet/common/Protocol.h"

namespace inet {
namespace coap {

const uint16_t COAP_UDP_PORT = 5683;

/**
 * Returns the feature-local Protocol descriptor for CoAP over UDP.
 * Constructed on first use during pre-network setup so Protocol::getNextId()
 * ordering for core protocols is unchanged.
 */
INET_API const Protocol& getCoapProtocol();

} // namespace coap
} // namespace inet

#endif
