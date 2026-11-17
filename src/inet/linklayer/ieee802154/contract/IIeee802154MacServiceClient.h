// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_IIEEE802154MACSERVICECLIENT_H
#define __INET_IIEEE802154MACSERVICECLIENT_H

#include "inet/common/INETDefs.h"
#include "inet/linklayer/ieee802154/Ieee802154MacDataRequest.h"
#include "inet/linklayer/ieee802154/Ieee802154MacGetRequest.h"

namespace inet {

class Packet;

/**
 * Pure client-side MCPS/MLME callback contract. Confirmation references are
 * borrowed only during the callback. Indication packets transfer to the
 * client, which must enter module context and take them immediately.
 * Providers detach state before callbacks, so ordinary callbacks may submit
 * reentrantly. Stop/crash reports STOPPED; reset reports RESET before its own
 * confirmation. Self-deletion during a callback is outside this contract;
 * a deleted client receives no later callback.
 */
class INET_API IIeee802154MacServiceClient
{
  public:
    virtual ~IIeee802154MacServiceClient() = default;

    virtual void handleDataConfirm(Ieee802154MacServiceRequestId requestId,
            const Ieee802154MacDataConfirm& confirm) = 0;
    virtual void handleGetConfirm(Ieee802154MacServiceRequestId requestId,
            const Ieee802154MacGetConfirm& confirm) = 0;
    virtual void handleSetConfirm(Ieee802154MacServiceRequestId requestId,
            const Ieee802154MacSetConfirm& confirm) = 0;
    virtual void handleResetConfirm(Ieee802154MacServiceRequestId requestId,
            const Ieee802154MacResetConfirm& confirm) = 0;

    /** The client takes and owns msdu/rawFrame; the provider does not reuse it. */
    virtual void handleDataIndication(Packet *msdu,
            const Ieee802154MacDataIndication& indication) = 0;
    virtual void handlePromiscuousIndication(Packet *rawFrame,
            const Ieee802154MacPromiscuousIndication& indication) = 0;
    virtual void handleCommStatusIndication(Packet *plaintext,
            const Ieee802154MacCommStatusIndication& indication) = 0;
};

} // namespace inet

#endif
