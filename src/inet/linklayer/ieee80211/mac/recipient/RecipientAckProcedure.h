//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_RECIPIENTACKPROCEDURE_H
#define __INET_RECIPIENTACKPROCEDURE_H

#include "inet/linklayer/ieee80211/mac/contract/IRecipientAckProcedure.h"

namespace inet {
namespace ieee80211 {

/*
 * This class implements 9.3.2.8 ACK procedure
 */
class INET_API RecipientAckProcedure : public IRecipientAckProcedure
{
  protected:
    int numReceivedAckableFrame = 0;
    int numSentAck = 0;

  protected:
    virtual const Ptr<Ieee80211AckFrame> buildAck(const Ptr<const Ieee80211DataOrMgmtHeader>& dataOrMgmtHeader) const;

  public:
    virtual void processReceivedFrame(Packet *packet, const Ptr<const Ieee80211DataOrMgmtHeader>& dataOrMgmtHeader, IRecipientAckPolicy *ackPolicy, IProcedureCallback *callback) override;
    virtual void processTransmittedAck(const Ptr<const Ieee80211AckFrame>& ack) override;
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif

