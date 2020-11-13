//
// Copyright (C) 2014 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#ifndef __INET_IARP_H
#define __INET_IARP_H

#include "inet/common/ModuleAccess.h"
#include "inet/linklayer/common/MacAddress.h"
#include "inet/networklayer/common/L3Address.h"
#include "inet/networklayer/contract/ipv4/Ipv4Address.h"

namespace inet {

class NetworkInterface;

/**
 * Represents an Ipv4 ARP module.
 */
class INET_API IArp
{
  public:
    /**
     * Sent in ARP cache change notification signals
     */
    class Notification : public cObject
    {
      public:
        L3Address l3Address;
        MacAddress macAddress;
        const NetworkInterface *ie;

      public:
        Notification(L3Address l3Address, MacAddress macAddress, const NetworkInterface *ie)
            : l3Address(l3Address), macAddress(macAddress), ie(ie) {}
    };

    /** @brief Signals used to publish ARP state changes. */
    static const simsignal_t arpResolutionInitiatedSignal;
    static const simsignal_t arpResolutionCompletedSignal;
    static const simsignal_t arpResolutionFailedSignal;

  public:
    virtual ~IArp() {}

    /**
     * Returns the Layer 3 address for the given MAC address. If it is not available
     * (not in the cache, pending resolution, or already expired), UNSPECIFIED_ADDRESS
     * is returned.
     */
    virtual L3Address getL3AddressFor(const MacAddress&) const = 0;

    /**
     * Tries to resolve the given network address to a MAC address. If the MAC
     * address is not yet resolved it returns an unspecified address and starts
     * an address resolution procedure. A signal is emitted when the address
     * resolution procedure terminates.
     */
    virtual MacAddress resolveL3Address(const L3Address& address, const NetworkInterface *ie) = 0;
};

} // namespace inet

#endif

