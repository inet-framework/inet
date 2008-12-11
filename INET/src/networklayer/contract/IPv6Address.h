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

#include <omnetpp.h>
#include <iostream>
#include <string>
#include "INETDefs.h"

class InterfaceToken;

/**
 * Stores an IPv6 address. Compliant to RFC 3513 - Internet Protocol Version 6
 * (IPv6) Addressing Architecture.
 *
 * Storage is efficient: an object occupies size of an IPv6 address
 * (128bits=16 bytes).
 */
class INET_API IPv6Address
{
    private:
        // Declare an unsigned 32 bit integer array of size 4.
        // 32x4 = 128 bits, which is the size of an IPv6 Address.
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
        //@}

        /**
         *  Constructor
         *  Set all 128 bits of the IPv6 address to '0'.
         *  0:0:0:0:0:0:0:0
         */
        IPv6Address()  {
            d[0]=d[1]=d[2]=d[3]=0;
        }

        /**
         *  Constructor.
         *  Constructs an IPv6 address based from the 4 given segments.
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
        IPv6Address(const char *addr) {set(addr);}

        bool operator<(const IPv6Address& addr) const {return compare(addr)<0;}
        bool operator>(const IPv6Address& addr) const {return compare(addr)>0;}
        bool operator==(const IPv6Address& addr) const {
            return d[3]==addr.d[3] && d[0]==addr.d[0] && d[1]==addr.d[1] && d[2]==addr.d[2]; // d[3] differs most often, compare it first
        }
        bool operator!=(const IPv6Address& addr) const {return !operator==(addr);}

        /**
         * Returns -1, 0 or 1.
         */
        int compare(const IPv6Address& addr) const  {
            return d[0]<addr.d[0] ? -1 : d[0]>addr.d[0] ? 1 :
                   d[1]<addr.d[1] ? -1 : d[1]>addr.d[1] ? 1 :
                   d[2]<addr.d[2] ? -1 : d[2]>addr.d[2] ? 1 :
                   d[3]<addr.d[3] ? -1 : d[3]>addr.d[3] ? 1 : 0;
        }

        /**
         *  Try parsing an IPv6 address.
         *  Return true if the string contained a well-formed IPv6 address,
         *  and false otherwise.
         *
         *  TBD: explain syntax (refer to RFC?)
         */
        bool tryParse(const char *addr);

        /**
         * FIXME
         */
         bool tryParseAddrWithPrefix(const char *addr, int& prefixLen);

        /**
         *  Sets the IPv6 address. Given a string.
         */
        void set(const char *addr);

        /**
         *  Get the IPv6 address as a "standard string".
         */
        std::string str() const;

        /**
         * Set the address to the given four 32-bit integers.
         */
        void set(uint32 d0, uint32 d1, uint32 d2, uint32 d3) {
            d[0]=d0; d[1]=d1; d[2]=d2; d[3]=d3;
        }

        /**
         * Returns pointer to internal binary representation of address,
         * four 32-bit unsigned integers.
         */
        uint32 *words() {return d;}

        /**
         * Returns pointer to internal binary representation of address,
         * four 32-bit unsigned integers.
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
         * Construct a 128 bit mask based on the prefix length.
         * Mask should point to an array of four 32-bit unsigned integers.
         */
        static void constructMask(int prefixLength, uint32* mask);

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
         * Create solicited-node multicast address for this address.
         * This function replaces the prefix with FF02:0:0:0:0:1:FF00:0/104.
         */
        IPv6Address formSolicitedNodeMulticastAddress() const {
            return IPv6Address(*this).setPrefix(SOLICITED_NODE_PREFIX, 104);
        };

        /**
         * RFC 3513: Section 2.6.1
         * The Subnet-Router anycast address is predefined.  Its format is as
         * follows:
         * <pre>
         * |                         n bits                 |   128-n bits   |
         * +------------------------------------------------+----------------+
         * |                   subnet prefix                | 00000000000000 |
         * +------------------------------------------------+----------------+
         * </pre>
         */
        // TODO revise doc, revise function! make static?  (Andras)
        IPv6Address formSubnetRouterAnycastAddress(int prefixLength) const {
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

/**
 * FIXME TBD turn it into a proper class
 */
/*
class INET_API IPv6AddressPrefix : public IPv6Address
{
    public:
        char length;
};
*/

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

