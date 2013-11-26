//
// Copyright (C) 2013 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_IARP_H_
#define __INET_IARP_H_

#include "Address.h"

/**
 * This purely virtual interface provides an abstraction for different ARPs.
 *
 * @author Levente Meszaros
 */
class INET_API IARP
{
  public:
    /** @brief A signal used to publish ARP state changes. */
    static simsignal_t arpStateChangedSignal;

  public:
    virtual ~IARP() { }

    /**
     * Tries to resolve the given network address to a MAC address. If the MAC
     * address is not yet resolved it returns an unspecified address and starts
     * an address resolution procedure. A signal is emitted when the address
     * resolution procedure terminates.
     */
    virtual MACAddress resolveMACAddress(Address& address) const = 0;
};

#endif
