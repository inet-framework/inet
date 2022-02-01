//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IORIGINATORBLOCKACKPROCEDURE_H
#define __INET_IORIGINATORBLOCKACKPROCEDURE_H

#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/linklayer/ieee80211/mac/common/Ieee80211Defs.h"

namespace inet {
namespace ieee80211 {

class INET_API IOriginatorBlockAckProcedure
{
  public:
    virtual ~IOriginatorBlockAckProcedure() {}

    virtual const Ptr<Ieee80211BlockAckReq> buildCompressedBlockAckReqFrame(const MacAddress& receiverAddress, Tid tid, SequenceNumberCyclic startingSequenceNumber) const = 0;
    virtual const Ptr<Ieee80211BlockAckReq> buildBasicBlockAckReqFrame(const MacAddress& receiverAddress, Tid tid, SequenceNumberCyclic startingSequenceNumber) const = 0;
};

} // namespace ieee80211
} // namespace inet

#endif

