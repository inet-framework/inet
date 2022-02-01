//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ORIGINATORBLOCKACKPROCEDURE_H
#define __INET_ORIGINATORBLOCKACKPROCEDURE_H

#include "inet/linklayer/ieee80211/mac/contract/IOriginatorBlockAckProcedure.h"

namespace inet {
namespace ieee80211 {

class INET_API OriginatorBlockAckProcedure : public IOriginatorBlockAckProcedure
{
  public:
    virtual const Ptr<Ieee80211BlockAckReq> buildCompressedBlockAckReqFrame(const MacAddress& receiverAddress, Tid tid, SequenceNumberCyclic startingSequenceNumber) const override;
    virtual const Ptr<Ieee80211BlockAckReq> buildBasicBlockAckReqFrame(const MacAddress& receiverAddress, Tid tid, SequenceNumberCyclic startingSequenceNumber) const override;
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif

