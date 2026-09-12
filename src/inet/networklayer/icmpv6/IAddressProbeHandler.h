//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_IADDRESSPROBEHANDLER_H
#define __INET_IADDRESSPROBEHANDLER_H

#include "inet/common/INETDefs.h"
#include "inet/networklayer/contract/ipv6/Ipv6Address.h"

namespace inet {

class NetworkInterface;

/**
 * Receives the outcome of an address probe started with
 * Ipv6NeighbourDiscovery::startAddressProbe().
 */
class INET_API IAddressProbeHandler {
public:
    virtual ~IAddressProbeHandler() = default;

    /**
     * Called at most once, when the probe of addr on ie ends. It is not called for a probe
     * abandoned with cancelAddressProbe(), or dropped because Neighbour Discovery stopped;
     * a handler that keeps state per probe must therefore be able to discard it unprompted.
     *
     * @param unique  true when no other node claimed the address, false when one defended it
     */
    virtual void addressProbeCompleted(const Ipv6Address& addr, NetworkInterface *ie, bool unique) = 0;
};

}  // namespace inet

#endif
