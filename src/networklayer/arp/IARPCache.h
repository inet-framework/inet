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

#ifndef __INET_IARPCACHE_H
#define __INET_IARPCACHE_H


#include "INETDefs.h"

#include "IPv4Address.h"
#include "MACAddress.h"
#include "ModuleAccess.h"

class InterfaceEntry;

/**
 * Represents an ARP cache.
 */
class INET_API IARPCache
{
  public:
    /**
     * Sent in ARP cache change notification signals
     */
    class Notification : public cObject
    {
      public:
        IPv4Address ipv4Address;
        MACAddress macAddress;
        const InterfaceEntry *ie;
      public:
        Notification(IPv4Address ipv4Address, MACAddress macAddress, const InterfaceEntry *ie)
                : ipv4Address(ipv4Address), macAddress(macAddress), ie(ie) {}
    };

  public:
    virtual ~IARPCache() {}

    /**
     * Returns the MAC address for the given IPv4 address. If it is not available
     * (not in the cache, pending resolution, or already expired), UNSPECIFIED_ADDRESS
     * is returned.
     */
    virtual MACAddress getMACAddressFor(const IPv4Address&) const = 0;

    /**
     * Returns the IPv4 address for the given MAC address. If it is not available
     * (not in the cache, pending resolution, or already expired), UNSPECIFIED_ADDRESS
     * is returned.
     */
    virtual IPv4Address getIPv4AddressFor(const MACAddress&) const = 0;

    /**
     * Starts address resolution for the given address on the given interface.
     * This should only be called if getMACAddressFor(addr) fails.
     */
    virtual void startAddressResolution(const IPv4Address&, const InterfaceEntry *ie) = 0;
};

class INET_API ARPCacheAccess : public ModuleAccess<IARPCache>
{
  public:
    ARPCacheAccess() : ModuleAccess<IARPCache>("arp") {}
};

#endif
