//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_IPACKETEXTRACTOR_H
#define __INET_IPACKETEXTRACTOR_H

#include <functional>

#include "inet/common/packet/Packet.h"

namespace inet {
namespace queueing {

/** Provides semantic pull accounting while extracting a selected packet. */
class INET_API IPacketExtractor
{
  public:
    /**
     * Predicates must be stable and side-effect free from an initial
     * findPacket() through the corresponding dequeuePacket() selection.
     * Composite extractors may evaluate the predicate multiple times and on
     * multiple candidates while preserving their scheduling policy.
     */
    using PacketPredicate = std::function<bool(const Packet *)>;

  public:
    virtual ~IPacketExtractor() {}
    virtual Packet *findPacket(const PacketPredicate& predicate) const = 0;
    virtual Packet *dequeuePacket(const PacketPredicate& predicate) = 0;
};

} // namespace queueing
} // namespace inet

#endif
