/*
 * Copyright (C) 2004 Andras Varga
 * Copyright (C) 2014 OpenSim Ltd.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __INET_IARP_H
#define __INET_IARP_H

#include "inet/common/INETDefs.h"

#include "inet/networklayer/common/L3Address.h"
#include "inet/networklayer/contract/ipv4/IPv4Address.h"
#include "inet/linklayer/common/MACAddress.h"
#include "inet/common/ModuleAccess.h"

namespace inet {

class InterfaceEntry;

/**
 * Represents an IPv4 ARP module.
 */
class INET_API IARP
{
  public:
    /**
     * Sent in ARP cache change notification signals
     */
    class Notification : public cObject
    {
      public:
        L3Address l3Address;
        MACAddress macAddress;
        const InterfaceEntry *ie;

      public:
        Notification(L3Address l3Address, MACAddress macAddress, const InterfaceEntry *ie)
            : l3Address(l3Address), macAddress(macAddress), ie(ie) {}
    };

    /** @brief Signals used to publish ARP state changes. */
    static const simsignal_t initiatedARPResolutionSignal;
    static const simsignal_t completedARPResolutionSignal;
    static const simsignal_t failedARPResolutionSignal;

  public:
    virtual ~IARP() {}

    /**
     * Returns the Layer 3 address for the given MAC address. If it is not available
     * (not in the cache, pending resolution, or already expired), UNSPECIFIED_ADDRESS
     * is returned.
     */
    virtual L3Address getL3AddressFor(const MACAddress&) const = 0;

    /**
     * Tries to resolve the given network address to a MAC address. If the MAC
     * address is not yet resolved it returns an unspecified address and starts
     * an address resolution procedure. A signal is emitted when the address
     * resolution procedure terminates.
     */
    virtual MACAddress resolveL3Address(const L3Address& address, const InterfaceEntry *ie) = 0;
};

} // namespace inet

#endif // ifndef __INET_IARP_H

