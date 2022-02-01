//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IRECIPIENTACKPROCEDURE_H
#define __INET_IRECIPIENTACKPROCEDURE_H

#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/linklayer/ieee80211/mac/contract/IProcedureCallback.h"
#include "inet/linklayer/ieee80211/mac/contract/IRecipientAckPolicy.h"

namespace inet {
namespace ieee80211 {

class INET_API IRecipientAckProcedure
{
  public:
    virtual ~IRecipientAckProcedure() {}

    virtual void processReceivedFrame(Packet *packet, const Ptr<const Ieee80211DataOrMgmtHeader>& dataOrMgmtHeader, IRecipientAckPolicy *ackPolicy, IProcedureCallback *callback) = 0;
    virtual void processTransmittedAck(const Ptr<const Ieee80211AckFrame>& ack) = 0;
};

} // namespace ieee80211
} // namespace inet

#endif

