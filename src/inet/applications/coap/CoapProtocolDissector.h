//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_COAP_COAPPROTOCOLDISSECTOR_H
#define __INET_COAP_COAPPROTOCOLDISSECTOR_H

#include "inet/common/packet/dissector/ProtocolDissector.h"

namespace inet {
namespace coap {

class INET_API CoapProtocolDissector : public ProtocolDissector
{
  public:
    virtual void dissect(Packet *packet, const Protocol *protocol, ICallback& callback) const override;
};

} // namespace coap
} // namespace inet

#endif
