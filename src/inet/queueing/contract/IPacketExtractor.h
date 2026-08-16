//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_IPACKETEXTRACTOR_H
#define __INET_IPACKETEXTRACTOR_H

#include "inet/common/packet/Packet.h"

namespace inet {
namespace queueing {

/** Provides semantic pull accounting while extracting a selected packet. */
class INET_API IPacketExtractor
{
  public:
    virtual ~IPacketExtractor() {}
    virtual Packet *dequeuePacket(Packet *packet) = 0;
};

} // namespace queueing
} // namespace inet

#endif
