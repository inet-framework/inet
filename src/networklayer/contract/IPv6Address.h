//
// Copyright (C) 2005 Wei Yang, Ng
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


#ifndef __IPv6ADDRESS_H
#define __IPv6ADDRESS_H

#include <iostream>
#include <string>

#include "INETDefs.h"

class InterfaceToken;

/**
 * Stores a 128-bit IPv6 address in an efficient way. Complies to RFC 3513,
 * "Internet Protocol Version 6 (IPv6) Addressing Architecture."
 */
class INET_API IPv6Address
{
    private:
        // The 128-bit address in four 32-bit integers. d[0] is the most
        // significant word, d[3] is the least significant one.
        uint32 d[4];

    protected:
        bool doTryParse(const char *&addr);

    public:
        /**
         * IPv6 address scope (RFC 3513)
         */
        // FIXME TBD add multicast address scopes!!! rfc 3513, section 2.7
        enum Scope
        {
            UNSPECIFIED,
            LOOPBACK,
            MULTICAST,
            LINK,
            SITE,
            GLOBAL
        };

        /** @name Predefined addresses */
        //@{
        /** The unspecified address */
        static const IPv6Address UNSPECIFIED_ADDRESS;

        /** The loopback address */
        static const IPv6Address LOOPBACK_ADDRESS;

        /** All-nodes multicast address, scope 1 (interface-local) */
        static const IPv6Address ALL_NODES_1;

        /** All-nodes multicast address, scope 2 (link-local) */
        static const IPv6Address ALL_NODES_2;

        /** All-routers multicast address, scope 1 (interface-local) */
        static const IPv6Address ALL_ROUTERS_1;

        /** All-routers multicast address, scope 2 (link-local) */
        static const IPv6Address ALL_ROUTERS_2;

        /** All-routers multicast address, scope 5 (site-local) */
        static const IPv6Address ALL_ROUTERS_5;

        /** The solicited-node multicast address prefix (prefix length = 104) */
        static const IPv6Address SOLICITED_NODE_PREFIX;

        /** The link-local prefix (fe80::) */
        static const IPv6Address LINKLOCAL_PREFIX;

        /** Link-local MANET routers multicast address */
        static const IPv6Address LL_MANET_ROUTERS;
        //@}

        /**
         * Constructor. Initializes the IPv6 address to ::0 (all-zeroes).
         */
        IPv6Address()  {d[0] = d[1] = d[2] = d[3] = 0;}

        /**
         * Constructs an IPv6 address from four 32-bit integers. The most significant
         * word should be passed in the first argument.
         */
        IPv6Address(uint32 segment0, uint32 segment1, uint32 segment2, uint32 segment3)  {
            d[0] = segment0;
            d[1] = segment1;
            d[2] = segment2;
            d[3] = segment3;
        }

        /**
         * Constructor. Sets the address from the given text representation.
         * See documentation of tryParse() for supported syntax.
         */
        explicit IPv6Address(const char *addr) {set(addr);}

        bool operator<(const IPv6Address& addr) const {return compare(addr) < 0;}
        bool operator>(const IPv6Address& addr) const {return compare(addr) > 0;}

        bool operator==(const IPv6Address& addr) const
        {
            return d[3]==addr.d[3] && d[2]==addr.d[2] && d[1]==addr.d[1] && d[0]==addr.d[0]; // d[3] differs most often, compare that first
        }

        bool operator!=(const IPv6Address& addr) const {return !operator==(addr);}

        /**
         * Returns -1, 0 or 1.
         */
        int compare(const IPv6Address& addr) const
        {
            return d[0] < addr.d[0] ? -1 : d[0] > addr.d[0] ? 1 :
                   d[1] < addr.d[1] ? -1 : d[1] > addr.d[1] ? 1 :
                   d[2] < addr.d[2] ? -1 : d[2] > addr.d[2] ? 1 :
                   d[3] < addr.d[3] ? -1 : d[3] > addr.d[3] ? 1 : 0;
        }

        /**
         * Tries parsing an IPv6 address string into the object.
         * Returns true if the string contains a well-formed IPv6 address,
         * and false otherwise. All RFC 3513 notations are accepted (e.g.
         * FEDC:BA98:7654:3210:FEDC:BA98:7654:3210, FF01::101, ::1), plus
         * also "<unspec>" as a synonym for the unspecified address (all-zeroes).
         */
        bool tryParse(const char *addr);

        /**
         * Expects a string in the "<address>/<prefixlength>" syntax, parses
         * the address into the object (see tryParse(), and returns the prefix
         * length (a 0..128 integer) in the second argument. The return value
         * is true if the operation was successful, and false if it was not
         * (e.g. no slash in the input string, invalid address syntax, prefix
         * length is out of range, etc.).
         */
        bool tryParseAddrWithPrefix(const char *addr, int& prefixLen);

        /**
         * Sets the IPv6 address. Given a string.
         */
        void set(const char *addr);

        /**
         * Returns the textual representation of the address in the standard
         * notation.
         */
        std::string str() const;

        /**
         * Sets the IPv6 address from four 32-bit integers. The most significant
         * word should be passed in the first argument.
         */
        void set(uint32 d0, uint32 d1, uint32 d2, uint32 d3) {
            d[0] = d0; d[1] = d1; d[2] = d2; d[3] = d3;
        }

        /**
         * Returns a pointer to the internal binary representation of the address:
         * four 32-bit words, most significant word first.
         */
        uint32 *words() {return d;}

        /**
         * Returns a pointer to the internal binary representation of the address:
         * four 32-bit words, most significant word first.
         */
        const uint32 *words() const {return d;}

        /**
         * Get the IPv6 address scope.
         */
        Scope getScope() const;

        /**
         * Return the string representation of the given scope.
         */
        static const char *scopeName(Scope s);

        /**
         * Construct a 128-bit mask based on the prefix length. Mask should point
         * to an array of four 32-bit words, most significant word first.
         */
        static void constructMask(int prefixLength, uint32* mask);
        static IPv6Address constructMask(int prefixLength);

        /**
         * Get the IPv6 first prefixLength bits of the address, with
         * the rest set to zero.
         */
        IPv6Address getPrefix(int prefixLength) const;

        /**
         * Get the last 128-prefixLength bits of the address, with the
         * first bits set to zero.
         */
        IPv6Address getSuffix(int prefixLength) const;

        /**
         * Overwrites the first prefixLength bits of the address with
         * the bits from the address passed as argument.
         * Return value is the object itself.
         */
        const IPv6Address& setPrefix(const IPv6Address& fromAddr, int prefixLength);

        /**
         * Overwrites the last 128-prefixLength bits of the address with
         * the bits from address passed as argument.
         * Return value is the object itself.
         */
        const IPv6Address& setSuffix(const IPv6Address& fromAddr, int prefixLength);

        /**
         * Returns the solicited-node multicast address for this address.
         * This function replaces the prefix with FF02:0:0:0:0:1:FF00:0/104.
         */
        IPv6Address formSolicitedNodeMulticastAddress() const {
            return IPv6Address(*this).setPrefix(SOLICITED_NODE_PREFIX, 104);
        };

        bool isSolicitedNodeMulticastAddress() const {
            return matches(SOLICITED_NODE_PREFIX, 104);
        }

        /**
         * Returns the subnet-router anycast address for this address by
         * setting its suffix (the last 128-prefixLength bits) to all-zeroes.
         * See section 2.6.1 of RFC 3513.
         */
        IPv6Address formSubnetRouterAnycastAddress(int prefixLength) const
        {
            return IPv6Address(*this).setSuffix(UNSPECIFIED_ADDRESS, prefixLength);
        }

        /**
         * Forms a link-local address using the given interface identifier.
         */
        static IPv6Address formLinkLocalAddress(const InterfaceToken& ident);

        /**
         * Returns true if the address matches the given prefix.
         */
        bool matches(const IPv6Address& prefix, int prefixLength) const;

        /**
         * Check if the IPv6 Address is undefined.
         */
        bool isUnspecified() const  {return (d[0]|d[1]|d[2]|d[3])==0;}

        /** Utility function based on getScope() */
        bool isMulticast() const {return getScope()==MULTICAST;}

        /** Utility function based on getScope() */
        bool isUnicast() const {return getScope()!=MULTICAST && getScope()!=UNSPECIFIED;}

        /** Utility function based on getScope() */
        bool isLoopback() const {return getScope()==LOOPBACK;}

        /** Utility function based on getScope() */
        bool isLinkLocal() const {return getScope()==LINK;}

        /** Utility function based on getScope() */
        bool isSiteLocal() const {return getScope()==SITE;}

        /** Utility function based on getScope() */
        bool isGlobal() const {return getScope()==GLOBAL;}

        /**
         * Get the 4-bit scope field of an IPv6 multicast address.
         */
        int getMulticastScope() const;
};

inline std::ostream& operator<<(std::ostream& os, const IPv6Address& ip)
{
    return os << ip.str();
}

inline void doPacking(cCommBuffer *buf, const IPv6Address& addr)
{
    buf->pack(addr.words(), 4);
}

inline void doUnpacking(cCommBuffer *buf, IPv6Address& addr)
{
    buf->unpack(addr.words(), 4);
}

#endif

