//
// Copyright (C) 2012 Andras Varga
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

#ifndef __INET_L3ADDRESS_H
#define __INET_L3ADDRESS_H

#include "inet/common/INETDefs.h"

#include "inet/networklayer/contract/ipv4/IPv4Address.h"
#include "inet/networklayer/contract/ipv6/IPv6Address.h"
#include "inet/linklayer/common/MACAddress.h"
#include "inet/networklayer/common/ModuleIdAddress.h"
#include "inet/networklayer/common/ModulePathAddress.h"

namespace inet {

class IL3AddressType;

/**
 * This class provides the generic interface for network addresses. For efficiency reasons the
 * implementation uses a 128 bits integer to represent all kinds of network addresses. The
 * different address implementations should not subclass this class, they should rather provide
 * conversions to and from this class.
 *
 * @see IPv4Address, IPv6Address, MACAddress, ModulePathAddress, ModuleIdAddress
 */
class INET_API L3Address
{
  public:
    enum AddressType {
        NONE,
        IPv4,
        IPv6,
        MAC,
        MODULEPATH,
        MODULEID
    };

  private:
    uint64 hi;
    uint64 lo;

  private:
    uint64 get(AddressType type) const;
    void set(AddressType type, uint64 lo);

  public:
    L3Address() { set(NONE, 0); }
    explicit L3Address(const char *str) { tryParse(str); }
    L3Address(const IPv4Address& addr) { set(addr); }
    L3Address(const IPv6Address& addr) { set(addr); }
    L3Address(const MACAddress& addr) { set(addr); }
    L3Address(const ModuleIdAddress& addr) { set(addr); }
    L3Address(const ModulePathAddress& addr) { set(addr); }

    void set(const IPv4Address& addr) { set(IPv4, addr.getInt()); }
    void set(const IPv6Address& addr);
    void set(const MACAddress& addr) { set(MAC, addr.getInt()); }
    void set(const ModuleIdAddress& addr) { set(MODULEID, addr.getId()); }
    void set(const ModulePathAddress& addr) { set(MODULEPATH, addr.getId()); }

    IPv4Address toIPv4() const { return getType() == NONE ? IPv4Address() : IPv4Address(get(IPv4)); }
    IPv6Address toIPv6() const { return getType() == NONE ? IPv6Address() : IPv6Address(hi, lo); }
    MACAddress toMAC() const { return getType() == NONE ? MACAddress() : MACAddress(get(MAC)); }
    ModuleIdAddress toModuleId() const { return getType() == NONE ? ModuleIdAddress() : ModuleIdAddress(get(MODULEID)); }
    ModulePathAddress toModulePath() const { return getType() == NONE ? ModulePathAddress() : ModulePathAddress(get(MODULEPATH)); }

    std::string str() const;
    AddressType getType() const;
    IL3AddressType *getAddressType() const;

    /**
     * Get the first prefixLength bits of the address, with the rest set to zero.
     */
    L3Address getPrefix(int prefixLength) const;

    bool tryParse(const char *addr);

    bool isUnspecified() const;
    bool isUnicast() const;
    bool isMulticast() const;
    bool isBroadcast() const;
    bool isLinkLocal() const;

    bool operator<(const L3Address& other) const;
    bool operator>(const L3Address& other) const { return other < *this; };
    bool operator==(const L3Address& other) const;
    bool operator!=(const L3Address& other) const;

    bool matches(const L3Address& other, int prefixLength) const;

    static const char *getTypeName(AddressType t);
};

inline std::ostream& operator<<(std::ostream& os, const L3Address& address)
{
    return os << address.str();
}

} // namespace inet

#endif // ifndef __INET_ADDRESS_H

