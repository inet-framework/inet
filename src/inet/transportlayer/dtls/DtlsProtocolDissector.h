// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef INET_DTLSPROTOCOLDISSECTOR_H
#define INET_DTLSPROTOCOLDISSECTOR_H

#include "inet/common/packet/dissector/ProtocolDissector.h"

namespace inet::dtls {

class INET_API DtlsProtocolDissector : public ProtocolDissector
{
  public:
    void dissect(Packet *packet, const Protocol *protocol, ICallback& callback) const override;
};

} // namespace inet::dtls
#endif
