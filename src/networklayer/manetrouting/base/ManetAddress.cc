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


#include "ManetAddress.h"


ManetAddress ManetAddress::ZERO;


void ManetAddress::set(const IPv6Address& addr)
{
    addrType = IPv6_ADDRESS;
    const uint32_t *w = addr.words();
    hi = ((uint64_t)w[0] << 32) + w[1];
    lo = ((uint64_t)w[2] << 32) + w[3];
}

void ManetAddress::set(const IPvXAddress& addr)
{
    if (addr.isIPv6())
        set(addr.get6());
    else
        set(addr.get4());
}

void ManetAddress::setPrefix(short unsigned int masklen)
{
    switch (addrType)
    {
        case IPv4_ADDRESS: hi &= IPv4Address::makeNetmask(masklen).getInt(); break;
        case IPv6_ADDRESS: set(_getIPv6().getPrefix(masklen)); break;
        case MAC_ADDRESS:  if (masklen != 48) throw cRuntimeError("mask not supported for MACAddress"); break;
        default: throw cRuntimeError("setPrefixLen(): Undefined address type"); break;
    }
}

IPv4Address ManetAddress::getIPv4() const
{
    if (addrType == IPv4_ADDRESS)
        return _getIPv4();
    if (addrType == UNDEFINED)
        return IPv4Address::UNSPECIFIED_ADDRESS;
    throw cRuntimeError("ManetAddress is not an IPv4Address");
}

IPv6Address ManetAddress::getIPv6() const
{
    if (addrType == IPv6_ADDRESS)
        return _getIPv6();
    if (addrType == UNDEFINED)
        return IPv6Address::UNSPECIFIED_ADDRESS;
    throw cRuntimeError("ManetAddress is not an IPv6Address");
}

IPvXAddress ManetAddress::getIPvX() const
{
    if (addrType == IPv4_ADDRESS)
        return IPvXAddress(_getIPv4());
    if (addrType == IPv6_ADDRESS)
        return IPvXAddress(_getIPv6());
    if (addrType == UNDEFINED)
        return IPvXAddress(IPv4Address::UNSPECIFIED_ADDRESS);
    throw cRuntimeError("ManetAddress is not an IPvXAddress");
}

MACAddress ManetAddress::getMAC() const
{
    if (addrType == MAC_ADDRESS)
        return _getMAC();
    if (addrType == UNDEFINED)
        return MACAddress::UNSPECIFIED_ADDRESS;
    throw cRuntimeError("ManetAddress is not a MACAddress");
}

bool ManetAddress::isBroadcast() const
{
    switch (addrType)
    {
        case IPv4_ADDRESS: return _getIPv4().isLimitedBroadcastAddress();
        case IPv6_ADDRESS: throw cRuntimeError("isBroadcast() not implemented in IPv6"); //FIXME !!!!!
        case MAC_ADDRESS:  return _getMAC().isBroadcast();
        default: break;
    }
    throw cRuntimeError("isBroadcast(): Undefined address type");
}

bool ManetAddress::isMulticast() const
{
    switch (addrType)
    {
        case IPv4_ADDRESS: return _getIPv4().isMulticast();
        case IPv6_ADDRESS: return _getIPv6().isMulticast();
        case MAC_ADDRESS:  return _getMAC().isMulticast();
        default: break;
    }
    throw cRuntimeError("isMulticast(): Undefined address type");
}

bool ManetAddress::isUnspecified() const
{
    switch (addrType)
    {
        case IPv4_ADDRESS: return _getIPv4().isUnspecified();
        case IPv6_ADDRESS: return _getIPv6().isUnspecified();
        case MAC_ADDRESS:  return _getMAC().isUnspecified();
        case UNDEFINED:    return true;
        default: break;
    }
    return true;
}

short int ManetAddress::compare(const ManetAddress& other) const
{
    if (addrType < other.addrType)
        return -1;
    if (addrType > other.addrType)
        return 1;
    if (hi < other.hi)
        return -1;
    if (hi > other.hi)
        return 1;
    if (lo < other.lo)
        return -1;
    if (lo > other.lo)
        return 1;
    return 0;
}

std::string ManetAddress::str() const
{
    switch (addrType)
    {
        case IPv4_ADDRESS: return _getIPv4().str();
        case IPv6_ADDRESS: return _getIPv6().str();
        case MAC_ADDRESS:  return _getMAC().str();
        case UNDEFINED: return "<undefined>";
        default: break;
    }
    throw cRuntimeError("str(): Unknown address type");
}


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

void ManetNetworkAddress::set(const IPvXAddress& addr)
{
    if (addr.isIPv6())
        set(addr.get6());
    else
        set(addr.get4());
}

void ManetNetworkAddress::set(const IPvXAddress& addr, short unsigned int masklen)
{
    if (addr.isIPv6())
        set(addr.get6(), masklen);
    else
        set(addr.get4(), masklen);
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
    address.setPrefix(masklen);
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
    return address.compare(other.address);
}

std::string ManetNetworkAddress::str() const
{
    std::ostringstream ss;
    ss << address << '/' << prefixLength;
    return ss.str();
}

bool ManetNetworkAddress::contains(const ManetAddress& other) const
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

