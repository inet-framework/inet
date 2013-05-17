//
// Copyright (C) 2005 Andras Varga
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

#include "Address.h"
#include "IPv4AddressPolicy.h"
#include "IPv6AddressPolicy.h"

IAddressPolicy * Address::getAddressPolicy() const {
    switch (type) {
        case Address::NONE:
            throw cRuntimeError("Address contains no value");
        case Address::IPv4:
            return &IPv4AddressPolicy::INSTANCE;
        case Address::IPv6:
            return &IPv6AddressPolicy::INSTANCE;
        default:
            throw cRuntimeError("Unknown type");
    }
}

std::string Address::str() const {
    switch (type) {
        case Address::NONE:
            return "<none>";
        case Address::IPv4:
            return toIPv4().str();
        case Address::IPv6:
            return toIPv6().str();
        default:
            throw cRuntimeError("Unknown type");
    }
}

bool Address::tryParse(const char *addr)
{
    if (IPv4Address::isWellFormed(addr)) {
        type = IPv4;
        set(IPv4Address(addr));
        return true;
    }
    else if (ipv6.tryParse(addr)) {
        type = IPv6;
        return true;
    }
    else
        return false;
}

bool Address::isUnspecified() const
{
    switch (type) {
        case Address::NONE:
            return true;
        case Address::IPv4:
            return ipv4.isUnspecified();
        case Address::IPv6:
            return ipv6.isUnspecified();
        default:
            throw cRuntimeError("Unknown type");
    }
}

bool Address::isUnicast() const
{
    switch (type) {
        case Address::NONE:
            throw cRuntimeError("Address contains no value");
        case Address::IPv4:
            return !ipv4.isMulticast() && !ipv4.isLimitedBroadcastAddress();
        case Address::IPv6:
            return ipv6.isUnicast();
        default:
            throw cRuntimeError("Unknown type");
    }
}

bool Address::isMulticast() const
{
    switch (type) {
        case Address::NONE:
            throw cRuntimeError("Address contains no value");
        case Address::IPv4:
            return ipv4.isMulticast();
        case Address::IPv6:
            return ipv6.isMulticast();
        default:
            throw cRuntimeError("Unknown type");
    }
}

bool Address::isBroadcast() const
{
    switch (type) {
        case Address::NONE:
            throw cRuntimeError("Address contains no value");
        case Address::IPv4:
            return ipv4.isLimitedBroadcastAddress();
        case Address::IPv6:
            throw cRuntimeError("IPv6 isBroadcast() unimplemented");
        default:
            throw cRuntimeError("Unknown type");
    }
}

bool Address::operator<(const Address& address) const
{
    if (type != address.type)
        return type < address.type;
    else {
        switch (type) {
            case Address::NONE:
                throw cRuntimeError("Address contains no value");
            case Address::IPv4:
                return ipv4 < address.ipv4;
            case Address::IPv6:
                return ipv6 < address.ipv6;
            default:
                throw cRuntimeError("Unknown type");
        }
    }
}

bool Address::operator==(const Address& address) const
{
    if (type != address.type)
        return false;
    else {
        switch (type) {
            case Address::NONE:
                return true;
            case Address::IPv4:
                return ipv4 == address.ipv4;
            case Address::IPv6:
                return ipv6 == address.ipv6;
            default:
                throw cRuntimeError("Unknown type");
        }
    }
}

bool Address::operator!=(const Address& address) const
{
    return !operator==(address);
}

bool Address::matches(const Address& other, int prefixLength) const
{
    switch (type) {
        case Address::NONE:
            throw cRuntimeError("Address contains no value");
        case Address::IPv4:
            return IPv4Address::maskedAddrAreEqual(ipv4, other.ipv4, IPv4Address::makeNetmask(prefixLength)); //FIXME !!!!!
        case Address::IPv6:
            return ipv6.matches(other.ipv6, prefixLength);
        default:
            throw cRuntimeError("Unknown type");
    }
}

const char *Address::getTypeName(AddressType t)
{
#define CASE(x) case x: return #x
    switch (t)
    {
        CASE(NONE);
        CASE(IPv4);
        CASE(IPv6);
        default: return "Unknown type";
    }
#undef CASE
}

