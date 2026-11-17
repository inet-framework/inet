// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_IIEEE802154MACSERVICEPROVIDER_H
#define __INET_IIEEE802154MACSERVICEPROVIDER_H

#include "inet/common/INETDefs.h"
#include "inet/linklayer/ieee802154/Ieee802154MacDataRequest.h"
#include "inet/linklayer/ieee802154/Ieee802154MacGetRequest.h"

namespace inet {

class Packet;

/**
 * Pure provider-side MCPS/MLME service contract. Binding belongs to modules.
 * Request metadata is borrowed only during the call. A refused DATA request
 * leaves its packet with the caller; ACCEPTED transfers it before return,
 * including before an inline callback, and the provider consumes or destroys
 * that packet exactly once before a confirmation. Malformed invocation
 * (null packet, invalid/reused ID) is a local refusal; an accepted semantic
 * limitation completes with its IEEE status. Accepted operations
 * complete exactly once unless the bound client disappeared, in which case
 * ownership is cleaned without calling the deleted module.
 */
class INET_API IIeee802154MacServiceProvider
{
  public:
    virtual ~IIeee802154MacServiceProvider() = default;

    /**
     * IDs are client-issued, nonzero, strictly increasing, and never reused
     * across primitive kinds, refusals, reset, stop, or restart. UINT64_MAX is
     * the final usable ID. The request reference is borrowed only for this
     * call; accepted deferred work copies it.
     */
    virtual Ieee802154MacServiceAdmission submitData(Ieee802154MacServiceRequestId requestId,
            Packet *msdu, const Ieee802154MacDataRequest& request) = 0;
    virtual Ieee802154MacServiceAdmission submitGet(Ieee802154MacServiceRequestId requestId,
            const Ieee802154MacGetRequest& request) = 0;
    virtual Ieee802154MacServiceAdmission submitSet(Ieee802154MacServiceRequestId requestId,
            const Ieee802154MacSetRequest& request) = 0;
    virtual Ieee802154MacServiceAdmission submitReset(Ieee802154MacServiceRequestId requestId,
            const Ieee802154MacResetRequest& request) = 0;
    /**
     * Reset is a barrier: its own ID returns NOT_CANCELLABLE once accepted;
     * DATA/GET/SET pending at the barrier receive RESET before reset confirm.
     * Reentrant submissions remain BUSY until the barrier is cleared before
     * the reset confirmation. Stop/crash receives precedence and produces
     * STOPPED instead; cleanup never calls a deleted client.
     */
    virtual Ieee802154MacServiceCancelResult cancel(Ieee802154MacServiceRequestId requestId) = 0;
};

} // namespace inet

#endif
