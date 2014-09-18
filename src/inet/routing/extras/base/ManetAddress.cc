//
// Copyright (C) 2012 OpenSim Ltd
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this program; if not, see <http://www.gnu.org/licenses/>.
//


#include "inet/routing/extras/base/ManetAddress.h"

namespace inet {

namespace inetmanet {

void ManetNetworkAddress::set(IPv4Address addr, short unsigned int masklen)
{
    IPv4Address maskedAddr = addr;
    maskedAddr.doAnd(IPv4Address::makeNetmask(masklen));
    address.set(maskedAddr);
    prefixLength = masklen;
}

void ManetNetworkAddress::set(const IPv6Address& addr, short unsigned int masklen)
{
    IPv6Address maskedAddr = addr.getPrefix(masklen);
    address.set(maskedAddr);
    prefixLength = masklen;
}

void ManetNetworkAddress::set(const L3Address& addr)
{
    if (addr.getType() == L3Address::IPv6)
        set(addr.toIPv6());
    else
        set(addr.toIPv4());
}

void ManetNetworkAddress::set(const L3Address& addr, short unsigned int masklen)
{
    if (addr.getType() == L3Address::IPv6)
        set(addr.toIPv6(), masklen);
    else
        set(addr.toIPv4(), masklen);
}

void ManetNetworkAddress::set(MACAddress addr, short unsigned int masklen)
{
    if (masklen != 48)
        throw cRuntimeError("mask not supported for MACAddress");
    address.set(addr);
    prefixLength = masklen;
}

void ManetNetworkAddress::setPrefixLen(short unsigned int masklen)
{
    address = address.getPrefix(masklen);
    prefixLength = masklen;
}

short int ManetNetworkAddress::compare(const ManetNetworkAddress& other) const
{
    if (getType() < other.getType())
        return -1;
    if (getType() > other.getType())
        return 1;
    if (prefixLength > other.prefixLength)
        return -1;
    if (prefixLength < other.prefixLength)
        return 1;
    if (address < other.address)
        return -1;
    if (other.address < address)
        return 1;
    return 0;
}

std::string ManetNetworkAddress::str() const
{
    std::ostringstream ss;
    ss << address << '/' << prefixLength;
    return ss.str();
}

bool ManetNetworkAddress::contains(const L3Address& other) const
{
    if (getType() == other.getType())
    {
        ManetNetworkAddress tmp(other, prefixLength);
        return (*this == tmp);
    }
    return false;
}

bool ManetNetworkAddress::contains(const ManetNetworkAddress& other) const
{
    if (getType() == other.getType() && prefixLength <= other.prefixLength)
    {
        ManetNetworkAddress tmp(other.address, prefixLength);
        return (*this == tmp);
    }
    return false;
}

} // namespace inetmanet

} // namespace inet


