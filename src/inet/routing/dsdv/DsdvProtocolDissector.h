// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef __INET_DSDVPROTOCOLDISSECTOR_H
#define __INET_DSDVPROTOCOLDISSECTOR_H
#include "inet/common/packet/dissector/ProtocolDissector.h"
namespace inet {
namespace dsdv {
class INET_API DsdvProtocolDissector : public ProtocolDissector
{
  public:
    virtual void dissect(Packet *packet, const Protocol *protocol, ICallback& callback) const override;
};
} // namespace dsdv
} // namespace inet
#endif
