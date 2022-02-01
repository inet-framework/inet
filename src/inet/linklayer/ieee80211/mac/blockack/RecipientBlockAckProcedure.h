//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_RECIPIENTBLOCKACKPROCEDURE_H
#define __INET_RECIPIENTBLOCKACKPROCEDURE_H

#include "inet/linklayer/ieee80211/mac/contract/IRecipientBlockAckProcedure.h"

namespace inet {
namespace ieee80211 {

/*
 * This class implements 9.3.2.9 BlockAck procedure
 */
class INET_API RecipientBlockAckProcedure : public IRecipientBlockAckProcedure
{
  protected:
    int numReceivedBlockAckReq = 0;
    int numSentBlockAck = 0;

  protected:
    virtual const Ptr<Ieee80211BlockAck> buildBlockAck(const Ptr<const Ieee80211BlockAckReq>& blockAckReq, RecipientBlockAckAgreement *agreement);

  public:
    virtual void processReceivedBlockAckReq(Packet *blockAckPacket, const Ptr<const Ieee80211BlockAckReq>& blockAckReq, IRecipientQosAckPolicy *ackPolicy, IRecipientBlockAckAgreementHandler *blockAckAgreementHandler, IProcedureCallback *callback) override;
    virtual void processTransmittedBlockAck(const Ptr<const Ieee80211BlockAck>& blockAck) override;
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif

