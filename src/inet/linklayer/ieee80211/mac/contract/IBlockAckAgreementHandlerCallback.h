//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IBLOCKACKAGREEMENTHANDLERCALLBACK_H
#define __INET_IBLOCKACKAGREEMENTHANDLERCALLBACK_H

#include "inet/common/INETDefs.h"

namespace inet {
namespace ieee80211 {

class OriginatorBlockAckAgreement;
class RecipientBlockAckAgreement;

class INET_API IBlockAckAgreementHandlerCallback
{
  public:
    virtual ~IBlockAckAgreementHandlerCallback() {}

    // Recompute the shared timer from the absolute deadlines in both agreement handlers.
    virtual void scheduleInactivityTimer() = 0;

    // Retire overdue agreements in both roles before timeout DELBA enters channel access.
    virtual void expireBlockAckAgreements() = 0;

    // The map no longer contains the agreement. The handler owns the borrowed payload until this call returns. The callback must not retain or modify the payload.
    virtual void originatorBlockAckAgreementDeleted(OriginatorBlockAckAgreement *agreement) = 0;
    virtual void recipientBlockAckAgreementDeleted(RecipientBlockAckAgreement *agreement) = 0;
};

} // namespace ieee80211
} // namespace inet

#endif

