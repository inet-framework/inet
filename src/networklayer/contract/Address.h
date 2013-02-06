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

#ifndef __INET_IPVXADDRESS_H
#define __INET_IPVXADDRESS_H

#include <string.h>

#include "INETDefs.h"

#include "IPv4Address.h"
#include "IPv6Address.h"

/**
 * Stores an IPv4 or an IPv6 address. This class should be used everywhere
 * in transport layer and up, to guarantee IPv4/IPv6 transparency.
 *
 * Storage: an object occupies size of an IPv6 address
 * (128 bits=16 bytes) plus an int.
 */
class INET_API Address
{
  public:
    enum AddressType {
        IPv4,
        IPv6,
    };
  private:
    uint32 d[4];
    AddressType type;

  public:
    /**
     * Constructor for IPv4 addresses.
     */
    Address() {type = IPv4; d[0] = 0;}

    /**
     * Constructor for IPv4 addresses.
     */
    Address(const IPv4Address& addr) {set(addr);}

    /**
     * Constructor for IPv6 addresses.
     */
    Address(const IPv6Address& addr) {set(addr);}

    /**
     * Accepts string representations supported by IPv4Address (dotted decimal
     * notation) and IPv6Address (hex string with colons). Throws an error
     * if the format is not recognized.
     */
    explicit Address(const char *addr) {set(addr);}

    /**
     * Copy constructor.
     */
    Address(const Address& addr) {set(addr);}

    /**
     * Destructor
     */
    ~Address() {}

    /**
     * Return address type
     */
    AddressType getType() const {return type;}

    /**
     * Get IPv4 address. Throws exception if this is an IPv6 address.
     */
    IPv4Address toIPv4() const
    {
        if (type == IPv6)
            throw cRuntimeError("Address: cannot return IPv6 address %s as IPv4", str().c_str());

        return IPv4Address(d[0]);
    }

    /**
     * Get IPv6 address. Throws exception if this is an IPv4 address.
     */
    IPv6Address toIPv6() const
    {
        if (type == IPv4)
        {
            if (d[0] == 0) // allow null address to be returned as IPv6
                return IPv6Address();

            throw cRuntimeError("Address: cannot return IPv4 address %s as IPv6", str().c_str());
        }

        return IPv6Address(d[0], d[1], d[2], d[3]);
    }

    /**
     * Set to an IPv4 address.
     */
    void set(const IPv4Address& addr)
    {
        type = IPv4;
        d[0] = addr.getInt();
    }

    /**
     * Set to an IPv6 address.
     */
    void set(const IPv6Address& addr)
    {
        if (addr.isUnspecified())
        {
            // we always represent nulls as IPv4 null
            type = IPv4; d[0] = 0;
            return;
        }

        type = IPv6;
        const uint32 *w = addr.words();
        d[0] = w[0]; d[1] = w[1]; d[2] = w[2]; d[3] = w[3];
    }

    /**
     * Assignment
     */
    void set(const Address& addr)
    {
        type = addr.type;
        d[0] = addr.d[0];

        if (type == IPv6)
        {
            d[1] = addr.d[1]; d[2] = addr.d[2]; d[3] = addr.d[3];
        }
    }

    /**
     * Accepts string representations supported by IPv4Address (dotted decimal
     * notation) and IPv6Address (hex string with colons). Throws an error
     * if the format is not recognized.
     */
    void set(const char *addr);

    /**
     * Assignment
     */
    Address& operator=(const IPv4Address& addr) {set(addr); return *this;}

    /**
     * Assignment
     */
    Address& operator=(const IPv6Address& addr) {set(addr); return *this;}

    /**
     * Assignment
     */
    Address& operator=(const Address& addr) {set(addr); return *this;}

    /**
     * Parses and assigns the given address and returns true if the string is
     * recognized by IPv4Address or IPv6Address, otherwise just returns false.
     */
    bool tryParse(const char *addr);

    /**
     * Returns the string representation of the address (e.g. "152.66.86.92")
     */
    std::string str() const
    {
        if (type == IPv6)
        {
            return toIPv6().str();
        }
        else if (d[0] == 0)
        {
            return std::string("<unspec>");
        }
        else
        {
            return toIPv4().str();
        }
    }

    /**
     * True if the structure has not been assigned any address yet.
     */
    bool isUnspecified() const
    {
        return type == IPv4 && d[0] == 0;
    }

    /**
     * True if the stored address is a multicast address.
     */
    bool isMulticast() const
    {
        return type==IPv6 ? toIPv6().isMulticast() : toIPv4().isMulticast();
    }

    /**
     * Returns length of internal binary representation of address,
     * (count of 32-bit unsigned integers.)
     */
    int wordCount() const {return type==IPv6 ? 4 : 1;}

    /**
     * Returns pointer to internal binary representation of address,
     * four 32-bit unsigned integers.
     */
    const uint32 *words() const {return d;}

    /**
     * Returns true if the two addresses are equal
     */
    bool equals(const IPv4Address& addr) const {
        return type == IPv4 && d[0] == addr.getInt();
    }

    /**
     * Returns true if the two addresses are equal
     */
    bool equals(const IPv6Address& addr) const
    {
        const uint32 *w = addr.words();
        return type==IPv6 ? (d[3] == w[3] && d[2] == w[2] && d[1] == w[1] && d[0] == w[0]) : (isUnspecified() && addr.isUnspecified());
    }

    /**
     * Returns true if the two addresses are equal
     */
    bool equals(const Address& addr) const
    {
        return (type == addr.type) && (d[0] == addr.d[0])
                && (type == IPv4 || (d[3] == addr.d[3] && d[2] == addr.d[2] && d[1] == addr.d[1]));
    }

    /**
     * Returns equals(addr).
     */
    bool operator==(const IPv4Address& addr) const {return equals(addr);}

    /**
     * Returns !equals(addr).
     */
    bool operator!=(const IPv4Address& addr) const {return !equals(addr);}

    /**
     * Returns equals(addr).
     */
    bool operator==(const IPv6Address& addr) const {return equals(addr);}

    /**
     * Returns !equals(addr).
     */
    bool operator!=(const IPv6Address& addr) const {return !equals(addr);}

    /**
     * Returns equals(addr).
     */
    bool operator==(const Address& addr) const {return equals(addr);}

    /**
     * Returns !equals(addr).
     */
    bool operator!=(const Address& addr) const {return !equals(addr);}

    /**
     * Compares two addresses.
     */
    bool operator<(const Address& addr) const
    {
        if (type != addr.type)
            return type == IPv4;
        else if (type == IPv4)
            return d[0] < addr.d[0];
        else
            return memcmp(&d, &addr.d, 16) < 0;  // this provides an ordering, though not surely the one one would expect
    }
};

inline std::ostream& operator<<(std::ostream& os, const Address& ip)
{
    return os << ip.str();
}

inline void doPacking(cCommBuffer *buf, const Address& addr)
{
    if (buf->packFlag(addr.getType()==Address::IPv6))
        doPacking(buf, addr.toIPv6());
    else
        doPacking(buf, addr.toIPv4());
}

inline void doUnpacking(cCommBuffer *buf, Address& addr)
{
    if (buf->checkFlag())
    {
        IPv6Address tmp;
        doUnpacking(buf, tmp);
        addr.set(tmp);
    }
    else
    {
        IPv4Address tmp;
        doUnpacking(buf, tmp);
        addr.set(tmp);
    }
}

#endif

