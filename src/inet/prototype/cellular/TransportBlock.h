//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_TRANSPORTBLOCK_H
#define __INET_TRANSPORTBLOCK_H

#include "inet/common/Protocol.h"
#include "inet/common/packet/Packet.h"

namespace inet {

namespace cellular {

/**
 * Creates the dummy transport blocks that the MACs send in their allocations.
 */
class INET_API TransportBlock
{
  public:
    // the receivers of the radio medium copy the protocol tag of the transmitted packet
    static const Protocol protocol;

    // the size that the allocation carries with the MCS, at least the header
    static B computeSize(int numPrbs, int numSymbols, int mcs);

    // a transport block that fills the allocation, tagged with its AllocationReq
    static Packet *create(const char *name, int ueIndex, int carrierIndex, int mcs, const std::vector<int>& prbs, int numSymbols, W powerPerPrb, int beam, int intendedReceiverId);
};

} // namespace cellular

} // namespace inet

#endif

