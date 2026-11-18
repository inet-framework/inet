// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef __INET_AODVSUBNETPROTOCOLDISSECTOR_H
#define __INET_AODVSUBNETPROTOCOLDISSECTOR_H

#include "inet/common/packet/dissector/ProtocolDissector.h"

namespace inet {

class INET_API AodvSubnetProtocolDissector : public ProtocolDissector
{
  public:
    void dissect(Packet *packet, const Protocol *protocol, ICallback& callback) const override;
};

} // namespace inet

#endif
