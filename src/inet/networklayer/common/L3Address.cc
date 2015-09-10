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

#include "inet/networklayer/common/L3Address.h"
#include "inet/networklayer/contract/ipv4/IPv4AddressType.h"
#include "inet/networklayer/contract/ipv6/IPv6AddressType.h"
#include "inet/linklayer/common/MACAddressType.h"
#include "inet/networklayer/common/ModuleIdAddressType.h"
#include "inet/networklayer/common/ModulePathAddressType.h"

namespace inet {

#define RESERVED_IPV6_ADDRESS_RANGE    0x8000 // IETF reserved address range 8000::/16 (extended)

uint64 L3Address::get(AddressType type) const
{
    if (getType() == type)
        return lo;
    else
        throw cRuntimeError("Address is not of the given type");
}

void L3Address::set(AddressType type, uint64 lo)
{
    this->hi = ((uint64)RESERVED_IPV6_ADDRESS_RANGE << 48) + (uint64)type;
    this->lo = lo;
}

void L3Address::set(const IPv6Address& addr)
{
    const uint32 *words = addr.words();
    hi = ((uint64) * (words + 0) << 32) + *(words + 1);
    lo = ((uint64) * (words + 2) << 32) + *(words + 3);
    if (getType() != IPv6)
        throw cRuntimeError("Cannot set IPv6 address");
}

L3Address::AddressType L3Address::getType() const
{
    if (hi >> 48 == RESERVED_IPV6_ADDRESS_RANGE)
        return (AddressType)(hi & 0xFF);
    else
        return L3Address::IPv6;
}

IL3AddressType *L3Address::getAddressType() const
{
    switch (getType()) {
        case L3Address::NONE:
            throw cRuntimeError("Address contains no value");

        case L3Address::IPv4:
            return &IPv4AddressType::INSTANCE;

        case L3Address::IPv6:
            return &IPv6AddressType::INSTANCE;

        case L3Address::MAC:
            return &MACAddressType::INSTANCE;

        case L3Address::MODULEID:
            return &ModuleIdAddressType::INSTANCE;

        case L3Address::MODULEPATH:
            return &ModulePathAddressType::INSTANCE;

        default:
            throw cRuntimeError("Unknown type");
    }
}

std::string L3Address::str() const
{
    switch (getType()) {
        case L3Address::NONE:
            return "<none>";

        case L3Address::IPv4:
            return toIPv4().str();

        case L3Address::IPv6:
            return toIPv6().str();

        case L3Address::MAC:
            return toMAC().str();

        case L3Address::MODULEID:
            return toModuleId().str();

        case L3Address::MODULEPATH:
            return toModulePath().str();

        default:
            throw cRuntimeError("Unknown type");
    }
}

bool L3Address::tryParse(const char *addr)
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

bool L3Address::isUnspecified() const
{
    switch (getType()) {
        case L3Address::NONE:
            return true;

        case L3Address::IPv4:
            return toIPv4().isUnspecified();

        case L3Address::IPv6:
            return toIPv6().isUnspecified();

        case L3Address::MAC:
            return toMAC().isUnspecified();

        case L3Address::MODULEID:
            return toModuleId().isUnspecified();

        case L3Address::MODULEPATH:
            return toModulePath().isUnspecified();

        default:
            throw cRuntimeError("Unknown type");
    }
}

bool L3Address::isUnicast() const
{
    switch (getType()) {
        case L3Address::NONE:
            throw cRuntimeError("Address contains no value");

        case L3Address::IPv4:
            return !toIPv4().isMulticast() && !toIPv4().isLimitedBroadcastAddress();    // TODO: move to IPv4Address

        case L3Address::IPv6:
            return toIPv6().isUnicast();

        case L3Address::MAC:
            return !toMAC().isBroadcast() && !toMAC().isMulticast();    // TODO: move to MACAddress

        case L3Address::MODULEID:
            return toModuleId().isUnicast();

        case L3Address::MODULEPATH:
            return toModulePath().isUnicast();

        default:
            throw cRuntimeError("Unknown type");
    }
}

bool L3Address::isMulticast() const
{
    switch (getType()) {
        case L3Address::NONE:
            throw cRuntimeError("Address contains no value");

        case L3Address::IPv4:
            return toIPv4().isMulticast();

        case L3Address::IPv6:
            return toIPv6().isMulticast();

        case L3Address::MAC:
            return toMAC().isMulticast();

        case L3Address::MODULEID:
            return toModuleId().isMulticast();

        case L3Address::MODULEPATH:
            return toModulePath().isMulticast();

        default:
            throw cRuntimeError("Unknown type");
    }
}

bool L3Address::isBroadcast() const
{
    switch (getType()) {
        case L3Address::NONE:
            throw cRuntimeError("Address contains no value");

        case L3Address::IPv4:
            return toIPv4().isLimitedBroadcastAddress();

        case L3Address::IPv6:
            return false;

        //throw cRuntimeError("IPv6 isBroadcast() unimplemented");
        case L3Address::MAC:
            return toMAC().isBroadcast();

        case L3Address::MODULEID:
            return toModuleId().isBroadcast();

        case L3Address::MODULEPATH:
            return toModulePath().isBroadcast();

        default:
            throw cRuntimeError("Unknown type");
    }
}

bool L3Address::isLinkLocal() const
{
    switch (getType()) {
        case L3Address::NONE:
            throw cRuntimeError("Address contains no value");

        case L3Address::IPv4:
            return false;

        case L3Address::IPv6:
            return toIPv6().isLinkLocal();

        case L3Address::MAC:
            return true;

        case L3Address::MODULEID:
            return false;

        case L3Address::MODULEPATH:
            return false;

        default:
            throw cRuntimeError("Unknown type");
    }
}

bool L3Address::operator<(const L3Address& other) const
{
    AddressType type = getType();
    AddressType otherType = other.getType();
    if (type != otherType)
        return type < otherType;
    else {
        switch (type) {
            case L3Address::NONE:
                throw cRuntimeError("Address contains no value");

            case L3Address::IPv4:
                return toIPv4() < other.toIPv4();

            case L3Address::IPv6:
                return toIPv6() < other.toIPv6();

            case L3Address::MAC:
                return toMAC() < other.toMAC();

            case L3Address::MODULEID:
                return toModuleId() < other.toModuleId();

            case L3Address::MODULEPATH:
                return toModulePath() < other.toModulePath();

            default:
                throw cRuntimeError("Unknown type");
        }
    }
}

bool L3Address::operator==(const L3Address& other) const
{
    AddressType type = getType();
    if (type != other.getType())
        return false;
    else {
        switch (type) {
            case L3Address::NONE:
                return true;

            case L3Address::IPv4:
                return toIPv4() == other.toIPv4();

            case L3Address::IPv6:
                return toIPv6() == other.toIPv6();

            case L3Address::MAC:
                return toMAC() == other.toMAC();

            case L3Address::MODULEID:
                return toModuleId() == other.toModuleId();

            case L3Address::MODULEPATH:
                return toModulePath() == other.toModulePath();

            default:
                throw cRuntimeError("Unknown type");
        }
    }
}

bool L3Address::operator!=(const L3Address& other) const
{
    return !operator==(other);
}

bool L3Address::matches(const L3Address& other, int prefixLength) const
{
    switch (getType()) {
        case L3Address::NONE:
            throw cRuntimeError("Address contains no value");

        case L3Address::IPv4:
            return IPv4Address::maskedAddrAreEqual(toIPv4(), other.toIPv4(), IPv4Address::makeNetmask(prefixLength));    //FIXME !!!!!

        case L3Address::IPv6:
            return toIPv6().matches(other.toIPv6(), prefixLength);

        case L3Address::MAC:
            return toMAC() == other.toMAC();

        case L3Address::MODULEID:
            return toModuleId() == other.toModuleId();

        case L3Address::MODULEPATH:
            return ModulePathAddress::maskedAddrAreEqual(toModulePath(), other.toModulePath(), prefixLength);

        default:
            throw cRuntimeError("Unknown type");
    }
}

L3Address L3Address::getPrefix(int prefixLength) const
{
    switch (getType()) {
        case L3Address::NONE:
            return *this;

        case L3Address::IPv4:
            return L3Address(toIPv4().getPrefix(prefixLength));

        case L3Address::IPv6:
            return L3Address(toIPv6().getPrefix(prefixLength));

        case L3Address::MAC:
            if (prefixLength != 48)
                throw cRuntimeError("mask not supported for MACAddress");
            return *this;

        case L3Address::MODULEID:
        case L3Address::MODULEPATH:
            return *this;

        default:
            throw cRuntimeError("getPrefix(): Unknown type");
    }
}

const char *L3Address::getTypeName(AddressType t)
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

