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

#include <omnetpp.h>
#include <string.h>
#include "INETDefs.h"
#include "IPAddress.h"
#include "IPv6Address.h"


/**
 * Stores an IPv4 or an IPv6 address. This class should be used everywhere
 * in transport layer and up, to guarantee IPv4/IPv6 transparence.
 *
 * Storage is efficient: an object occupies size of an IPv6 address
 * (128bits=16 bytes) plus a bool.
 */
class INET_API IPvXAddress
{
  protected:
    uint32 d[4];
    bool isv6;

  public:
    /** name Constructors, destructor */
    //@{
    /**
     * Constructor for IPv4 addresses.
     */
    IPvXAddress() {isv6 = false; d[0] = 0;}

    /**
     * Constructor for IPv4 addresses.
     */
    IPvXAddress(const IPAddress& addr) {set(addr);}

    /**
     * Constructor for IPv6 addresses.
     */
    IPvXAddress(const IPv6Address& addr) {set(addr);}

    /**
     * Accepts string representations suuported by IPAddress (dotted decimal
     * notation) and IPv6Address (hex string with colons). Throws an error
     * if the format is not recognized.
     */
    IPvXAddress(const char *addr) {set(addr);}

    /**
     * Copy constructor.
     */
    IPvXAddress(const IPvXAddress& addr) {set(addr);}

    /**
     * Destructor
     */
    ~IPvXAddress() {}
    //@}

    /** name Getters, setters */
    //@{
    /**
     * Is this an IPv6 address?
     */
    bool isIPv6() const {return isv6;}

    /**
     * Get IPv4 address. Throws exception if this is an IPv6 address.
     */
    IPAddress get4() const {
        if (isv6)
            throw cRuntimeError("IPvXAddress: cannot return IPv6 address %s as IPv4", str().c_str());
        return IPAddress(d[0]);
    }

    /**
     * Get IPv6 address. Throws exception if this is an IPv4 address.
     */
    IPv6Address get6() const {
        if (!isv6)  {
            if (d[0]==0) // allow null address to be returned as IPv6
                return IPv6Address();
            throw cRuntimeError("IPvXAddress: cannot return IPv4 address %s as IPv6", str().c_str());
        }
        return IPv6Address(d[0], d[1], d[2], d[3]);
    }

    /**
     * Set to an IPv4 address.
     */
    void set(const IPAddress& addr)  {
        isv6 = false;
        d[0] = addr.getInt();
    }

    /**
     * Set to an IPv6 address.
     */
    void set(const IPv6Address& addr)  {
        if (addr.isUnspecified()) {
            // we always represent nulls as IPv4 null
            isv6 = false; d[0] = 0;
            return;
        }
        isv6 = true;
        uint32 *w = const_cast<IPv6Address&>(addr).words();
        d[0] = w[0]; d[1] = w[1]; d[2] = w[2]; d[3] = w[3];
    }

    /**
     * Assignment
     */
    void set(const IPvXAddress& addr) {
        isv6 = addr.isv6;
        d[0] = addr.d[0];
        if (isv6) {
            d[1] = addr.d[1]; d[2] = addr.d[2]; d[3] = addr.d[3];
         }
    }

    /**
     * Accepts string representations supported by IPAddress (dotted decimal
     * notation) and IPv6Address (hex string with colons). Throws an error
     * if the format is not recognized.
     */
    void set(const char *addr) {
        if (!tryParse(addr))
            throw cRuntimeError("IPvXAddress: cannot interpret address string `%s'", addr);
    }

    /**
     * Assignment
     */
    IPvXAddress& operator=(const IPAddress& addr) {set(addr); return *this;}

    /**
     * Assignment
     */
    IPvXAddress& operator=(const IPv6Address& addr) {set(addr); return *this;}

    /**
     * Assignment
     */
    IPvXAddress& operator=(const IPvXAddress& addr) {set(addr); return *this;}

    /**
     * Parses and assigns the given address and returns true if the string is
     * recognized by IPAddress or IPv6Address, otherwise just returns false.
     */
    bool tryParse(const char *addr);

    /**
     * Returns the string representation of the address (e.g. "152.66.86.92")
     */
    std::string str() const {return isv6 ? get6().str() : get4().str();}
    //@}

    /** name Comparison */
    //@{
    /**
     * True if the structure has not been assigned any address yet.
     */
    bool isUnspecified() const {
        return !isv6 && d[0]==0;
    }

    /**
     * Returns length of internal binary representation of address,
     * (count of 32-bit unsigned integers.)
     */
    int wordCount() const {return isv6 ? 4 : 1;}

    /**
     * Returns pointer to internal binary representation of address,
     * four 32-bit unsigned integers.
     */
    const uint32 *words() const {return d;}

    /**
     * Returns true if the two addresses are equal
     */
    bool equals(const IPAddress& addr) const {
        return !isv6 && d[0]==addr.getInt();
    }

    /**
     * Returns true if the two addresses are equal
     */
    bool equals(const IPv6Address& addr) const {
        uint32 *w = const_cast<IPv6Address&>(addr).words();
        return isv6 && d[0]==w[0] && d[1]==w[1] && d[2]==w[2] && d[3]==w[3];
    }

    /**
     * Returns true if the two addresses are equal
     */
    bool equals(const IPvXAddress& addr) const {
        return (isv6 == addr.isv6) && (d[0]==addr.d[0]) && (!isv6 || (d[1]==addr.d[1] && d[2]==addr.d[2] && d[3]==addr.d[3]));
    }

    /**
     * Returns equals(addr).
     */
    bool operator==(const IPAddress& addr) const {return equals(addr);}

    /**
     * Returns !equals(addr).
     */
    bool operator!=(const IPAddress& addr) const {return !equals(addr);}

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
    bool operator==(const IPvXAddress& addr) const {return equals(addr);}

    /**
     * Returns !equals(addr).
     */
    bool operator!=(const IPvXAddress& addr) const {return !equals(addr);}

    /**
     * Compares two addresses.
     */
    bool operator<(const IPvXAddress& addr) const {
        if (isv6!=addr.isv6)
            return !isv6;
        else if (!isv6)
            return d[0]<addr.d[0];
        else
            return memcmp(&d, &addr.d, 16) < 0;  // this provides an ordering, though not surely the one one would expect
    }
    //@}
};

inline std::ostream& operator<<(std::ostream& os, const IPvXAddress& ip)
{
    return os << ip.str();
}

inline void doPacking(cCommBuffer *buf, const IPvXAddress& addr)
{
    if (buf->packFlag(addr.isIPv6()))
        doPacking(buf, addr.get6());
    else
        doPacking(buf, addr.get4());
}

inline void doUnpacking(cCommBuffer *buf, IPvXAddress& addr)
{
    if (buf->checkFlag()) {
        IPv6Address tmp;
        doUnpacking(buf, tmp);
        addr.set(tmp);
    }
    else {
        IPAddress tmp;
        doUnpacking(buf, tmp);
        addr.set(tmp);
    }
}

#endif


