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
#include "IPv4AddressType.h"
#include "IPv6AddressType.h"
#include "MACAddressType.h"
#include "ModuleIdAddressType.h"
#include "ModulePathAddressType.h"

namespace inet {
#define RESERVED_IPV6_ADDRESS_RANGE    0x8000 // IETF reserved address range 8000::/16 (extended)

uint64 Address::get(AddressType type) const
{
    if (getType() == type)
        return lo;
    else
        throw cRuntimeError("Address is not of the given type");
}

void Address::set(AddressType type, uint64 lo)
{
    this->hi = ((uint64)RESERVED_IPV6_ADDRESS_RANGE << 48) + (uint64)type;
    this->lo = lo;
}

void Address::set(const IPv6Address& addr)
{
    const uint32 *words = addr.words();
    hi = ((uint64) * (words + 0) << 32) + *(words + 1);
    lo = ((uint64) * (words + 2) << 32) + *(words + 3);
    if (getType() != IPv6)
        throw cRuntimeError("Cannot set IPv6 address");
}

Address::AddressType Address::getType() const
{
    if (hi >> 48 == RESERVED_IPV6_ADDRESS_RANGE)
        return (AddressType)(hi & 0xFF);
    else
        return Address::IPv6;
}

IAddressType *Address::getAddressType() const
{
    switch (getType()) {
        case Address::NONE:
            throw cRuntimeError("Address contains no value");

        case Address::IPv4:
            return &IPv4AddressType::INSTANCE;

        case Address::IPv6:
            return &IPv6AddressType::INSTANCE;

        case Address::MAC:
            return &MACAddressType::INSTANCE;

        case Address::MODULEID:
            return &ModuleIdAddressType::INSTANCE;

        case Address::MODULEPATH:
            return &ModulePathAddressType::INSTANCE;

        default:
            throw cRuntimeError("Unknown type");
    }
}

std::string Address::str() const
{
    switch (getType()) {
        case Address::NONE:
            return "<none>";

        case Address::IPv4:
            return toIPv4().str();

        case Address::IPv6:
            return toIPv6().str();

        case Address::MAC:
            return toMAC().str();

        case Address::MODULEID:
            return toModuleId().str();

        case Address::MODULEPATH:
            return toModulePath().str();

        default:
            throw cRuntimeError("Unknown type");
    }
}

bool Address::tryParse(const char *addr)
{
    IPv6Address ipv6;
    MACAddress mac;
    ModuleIdAddress moduleId;
    ModulePathAddress modulePath;
    if (IPv4Address::isWellFormed(addr))
        set(IPv4Address(addr));
    else if (ipv6.tryParse(addr))
        set(ipv6);
    else if (mac.tryParse(addr))
        set(mac);
    else if (moduleId.tryParse(addr))
        set(moduleId);
    else if (modulePath.tryParse(addr))
        set(modulePath);
    else
        return false;
    return true;
}

bool Address::isUnspecified() const
{
    switch (getType()) {
        case Address::NONE:
            return true;

        case Address::IPv4:
            return toIPv4().isUnspecified();

        case Address::IPv6:
            return toIPv6().isUnspecified();

        case Address::MAC:
            return toMAC().isUnspecified();

        case Address::MODULEID:
            return toModuleId().isUnspecified();

        case Address::MODULEPATH:
            return toModulePath().isUnspecified();

        default:
            throw cRuntimeError("Unknown type");
    }
}

bool Address::isUnicast() const
{
    switch (getType()) {
        case Address::NONE:
            throw cRuntimeError("Address contains no value");

        case Address::IPv4:
            return !toIPv4().isMulticast() && !toIPv4().isLimitedBroadcastAddress();    // TODO: move to IPv4Address

        case Address::IPv6:
            return toIPv6().isUnicast();

        case Address::MAC:
            return !toMAC().isBroadcast() && !toMAC().isMulticast();    // TODO: move to MACAddress

        case Address::MODULEID:
            return toModuleId().isUnicast();

        case Address::MODULEPATH:
            return toModulePath().isUnicast();

        default:
            throw cRuntimeError("Unknown type");
    }
}

bool Address::isMulticast() const
{
    switch (getType()) {
        case Address::NONE:
            throw cRuntimeError("Address contains no value");

        case Address::IPv4:
            return toIPv4().isMulticast();

        case Address::IPv6:
            return toIPv6().isMulticast();

        case Address::MAC:
            return toMAC().isMulticast();

        case Address::MODULEID:
            return toModuleId().isMulticast();

        case Address::MODULEPATH:
            return toModulePath().isMulticast();

        default:
            throw cRuntimeError("Unknown type");
    }
}

bool Address::isBroadcast() const
{
    switch (getType()) {
        case Address::NONE:
            throw cRuntimeError("Address contains no value");

        case Address::IPv4:
            return toIPv4().isLimitedBroadcastAddress();

        case Address::IPv6:
            return false;

        //throw cRuntimeError("IPv6 isBroadcast() unimplemented");
        case Address::MAC:
            return toMAC().isBroadcast();

        case Address::MODULEID:
            return toModuleId().isBroadcast();

        case Address::MODULEPATH:
            return toModulePath().isBroadcast();

        default:
            throw cRuntimeError("Unknown type");
    }
}

bool Address::isLinkLocal() const
{
    switch (getType()) {
        case Address::NONE:
            throw cRuntimeError("Address contains no value");

        case Address::IPv4:
            return false;

        case Address::IPv6:
            return toIPv6().isLinkLocal();

        case Address::MAC:
            return true;

        case Address::MODULEID:
            return false;

        case Address::MODULEPATH:
            return false;

        default:
            throw cRuntimeError("Unknown type");
    }
}

bool Address::operator<(const Address& other) const
{
    AddressType type = getType();
    AddressType otherType = other.getType();
    if (type != otherType)
        return type < otherType;
    else {
        switch (type) {
            case Address::NONE:
                throw cRuntimeError("Address contains no value");

            case Address::IPv4:
                return toIPv4() < other.toIPv4();

            case Address::IPv6:
                return toIPv6() < other.toIPv6();

            case Address::MAC:
                return toMAC() < other.toMAC();

            case Address::MODULEID:
                return toModuleId() < other.toModuleId();

            case Address::MODULEPATH:
                return toModulePath() < other.toModulePath();

            default:
                throw cRuntimeError("Unknown type");
        }
    }
}

bool Address::operator==(const Address& other) const
{
    AddressType type = getType();
    if (type != other.getType())
        return false;
    else {
        switch (type) {
            case Address::NONE:
                return true;

            case Address::IPv4:
                return toIPv4() == other.toIPv4();

            case Address::IPv6:
                return toIPv6() == other.toIPv6();

            case Address::MAC:
                return toMAC() == other.toMAC();

            case Address::MODULEID:
                return toModuleId() == other.toModuleId();

            case Address::MODULEPATH:
                return toModulePath() == other.toModulePath();

            default:
                throw cRuntimeError("Unknown type");
        }
    }
}

bool Address::operator!=(const Address& other) const
{
    return !operator==(other);
}

bool Address::matches(const Address& other, int prefixLength) const
{
    switch (getType()) {
        case Address::NONE:
            throw cRuntimeError("Address contains no value");

        case Address::IPv4:
            return IPv4Address::maskedAddrAreEqual(toIPv4(), other.toIPv4(), IPv4Address::makeNetmask(prefixLength));    //FIXME !!!!!

        case Address::IPv6:
            return toIPv6().matches(other.toIPv6(), prefixLength);

        case Address::MAC:
            return toMAC() == other.toMAC();

        case Address::MODULEID:
            return toModuleId() == other.toModuleId();

        case Address::MODULEPATH:
            return ModulePathAddress::maskedAddrAreEqual(toModulePath(), other.toModulePath(), prefixLength);

        default:
            throw cRuntimeError("Unknown type");
    }
}

Address Address::getPrefix(int prefixLength) const
{
    switch (getType()) {
        case Address::NONE:
            return *this;

        case Address::IPv4:
            return Address(toIPv4().getPrefix(prefixLength));

        case Address::IPv6:
            return Address(toIPv6().getPrefix(prefixLength));

        case Address::MAC:
            if (prefixLength != 48)
                throw cRuntimeError("mask not supported for MACAddress");
            return *this;

        case Address::MODULEID:
        case Address::MODULEPATH:
            return *this;

        default:
            throw cRuntimeError("getPrefix(): Unknown type");
    }
}

const char *Address::getTypeName(AddressType t)
{
#define CASE(x)    case x: \
        return #x
    switch (t) {
        CASE(NONE);
        CASE(IPv4);
        CASE(IPv6);
        CASE(MAC);
        CASE(MODULEID);
        CASE(MODULEPATH);

        default:
            return "Unknown type";
    }
#undef CASE
}
} // namespace inet

