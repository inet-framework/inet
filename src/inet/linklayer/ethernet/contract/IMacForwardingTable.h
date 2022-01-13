//
// Copyright (C) 2013 OpenSim Ltd.
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

#ifndef __INET_IMACFORWARDINGTABLE_H
#define __INET_IMACFORWARDINGTABLE_H

#include "inet/linklayer/common/MacAddress.h"

namespace inet {

/**
 * This interface provides abstraction for a MAC forwarding information database.
 */
class INET_API IMacForwardingTable
{
  public:
    virtual int getUnicastAddressForwardingInterface(const MacAddress& address, unsigned int vid = 0) const = 0;
    virtual void setUnicastAddressForwardingInterface(int interfaceId, const MacAddress& address, unsigned int vid = 0) = 0;
    virtual void removeUnicastAddressForwardingInterface(int interfaceId, const MacAddress& address, unsigned int vid = 0) = 0;
    virtual void learnUnicastAddressForwardingInterface(int interfaceId, const MacAddress& address, unsigned int vid = 0) = 0;

    virtual std::vector<int> getMulticastAddressForwardingInterfaces(const MacAddress& address, unsigned int vid = 0) const = 0;
    virtual void addMulticastAddressForwardingInterface(int interfaceId, const MacAddress& address, unsigned int vid = 0) = 0;
    virtual void removeMulticastAddressForwardingInterface(int interfaceId, const MacAddress& address, unsigned int vid = 0) = 0;

    virtual void setAgingTime(simtime_t agingTime) = 0;

    /**
     *  @brief Clears portno cache
     */
    virtual void flush(int portno) = 0;

    /**
     * @brief Copy cache from portA to portB port
     */
    virtual void copyTable(int portA, int portB) = 0;
};

} // namespace inet

#endif

