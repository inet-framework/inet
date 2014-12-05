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

#include <iostream>
#include <sstream>
#include "inet/networklayer/contract/ipv6/IPv6Address.h"
#include "inet/networklayer/common/InterfaceToken.h"

namespace inet {

const uint32 LINK_LOCAL_PREFIX = 0xFE800000;
const uint32 SITE_LOCAL_PREFIX = 0xFEC00000;
const uint32 MULTICAST_PREFIX = 0xFF000000;

// Link and Site local masks should only preserve 10 bits as prefix length is 10.
const uint32 LINK_LOCAL_MASK = 0xFFC00000;
const uint32 SITE_LOCAL_MASK = 0xFFC00000;
const uint32 MULTICAST_MASK = 0xFF000000;

// RFC 3513: IPv6 Addressing Architecture
// Section 2.7.1: Pre-defined Multicast Addresses
const IPv6Address IPv6Address::UNSPECIFIED_ADDRESS("::0");
const IPv6Address IPv6Address::LOOPBACK_ADDRESS("::1");
const IPv6Address IPv6Address::ALL_NODES_1("FF01::1");
const IPv6Address IPv6Address::ALL_NODES_2("FF02::1");
const IPv6Address IPv6Address::ALL_ROUTERS_1("FF01::2");
const IPv6Address IPv6Address::ALL_ROUTERS_2("FF02::2");
const IPv6Address IPv6Address::ALL_ROUTERS_5("FF05::2");
const IPv6Address IPv6Address::SOLICITED_NODE_PREFIX("FF02:0:0:0:0:1:FF00:0");
const IPv6Address IPv6Address::LINKLOCAL_PREFIX("FE80::");
const IPv6Address IPv6Address::LL_MANET_ROUTERS("FF02:0:0:0:0:0:0:6D");

// Helper: Parses at most 8 colon-separated 16-bit hex numbers ("groups"),
// and returns their count. Advances s just over the last hex digit converted.
static int parseGroups(const char *& s, uint16_t *groups)
{
    int k = 0;
    while (1) {
        char *e;
        unsigned long grp = strtoul(s, &e, 16);
        if (s == e) {    // no hex digit converted
            if (k != 0)
                s--; // "unskip" preceding ':'
            break;
        }
        // if negative or too big, return (s will point to beginning of large number)
        if (grp > 0xffff)
            break;
        groups[k++] = grp;    // group[k] successfully stored
        s = e;    // skip converted hex number
        if (*s != ':' || k == 8)
            break;
        s++;    // skip ':'
    }
    return k;
}

bool IPv6Address::doTryParse(const char *& addr)
{
    if (!strcmp(addr, "<unspec>")) {
        addr += 8;
        d[0] = d[1] = d[2] = d[3] = 0;
        return true;
    }

    // parse and store 16-bit units
    uint16_t groups[8];
    int numGroups = parseGroups(addr, groups);

    // if address string contains "::", parse and store second half too
    if (*addr == ':' && *(addr + 1) == ':') {
        addr += 2;
        uint16_t suffixGroups[8];
        int numSuffixGroups = parseGroups(addr, suffixGroups);

        // merge suffixGroups[] into groups[]
        if (numGroups + numSuffixGroups > 8)
            return false; // too many
        for (int i = numGroups; i < 8; i++) {
            int j = i - 8 + numSuffixGroups;
            groups[i] = j < 0 ? 0 : suffixGroups[j];
        }
        numGroups = 8;
    }

    if (numGroups != 8)
        return false; // too few

    // copy groups to d[]
    for (unsigned int i = 0; i < 4; i++)
        d[i] = ((uint32_t)(groups[2 * i]) << 16) + groups[2 * i + 1];

    return true;
}

bool IPv6Address::tryParse(const char *addr)
{
    if (!addr)
        return false;
    if (!doTryParse(addr))
        return false;
    if (*addr != 0)
        return false; // illegal trailing character
    return true;
}

bool IPv6Address::tryParseAddrWithPrefix(const char *addr, int& prefixLen)
{
    if (!addr)
        return false;
    if (!doTryParse(addr))
        return false;
    if (*addr != '/')
        return false; // no '/' after address
    addr++;

    // parse prefix
    char *e;
    prefixLen = strtoul(addr, &e, 10);
    if (addr == e)
        return false; // no number after '/'
    if (*e != 0)
        return false; // garbage after number
    if (prefixLen < 0 || prefixLen > 128)
        return false; // wrong len value
    return true;
}

void IPv6Address::set(const char *addr)
{
    if (!tryParse(addr))
        throw cRuntimeError("IPv6Address: cannot interpret address string `%s'", addr);
}

// Helper: finds the longest sequence of zeroes in the address (at least with len=2)
static void findGap(uint16_t *groups, int& start, int& end)
{
    start = end = 0;
    int beg = -1;
    for (int i = 0; i < 8; i++) {
        if (beg == -1 && groups[i] == 0) {
            // begin counting
            beg = i;
        }
        else if (beg != -1 && groups[i] != 0) {
            // end counting
            if (i - beg >= 2 && i - beg > end - start) {
                start = beg;
                end = i;
            }
            beg = -1;
        }
    }

    // check last zero-seq
    if (beg != -1 && beg <= 6 && 8 - beg > end - start) {
        start = beg;
        end = 8;
    }
}

std::string IPv6Address::str() const
{
    if (isUnspecified())
        return std::string("<unspec>");

    // convert to 16-bit grops
    uint16_t groups[8] = {
        uint16_t(d[0] >> 16), uint16_t(d[0] & 0xffff), uint16_t(d[1] >> 16), uint16_t(d[1] & 0xffff),
        uint16_t(d[2] >> 16), uint16_t(d[2] & 0xffff), uint16_t(d[3] >> 16), uint16_t(d[3] & 0xffff)
    };

    // find longest sequence of zeros in groups[]
    int start, end;
    findGap(groups, start, end);
    if (start == 0 && end == 8)
        return "::0"; // the unspecified address is a special case

    // print groups, replacing gap with "::"
    std::stringstream os;
    os << std::hex;
    for (int i = 0; i < start; i++)
        os << (i == 0 ? "" : ":") << groups[i];
    if (start != end)
        os << "::";
    for (int j = end; j < 8; j++)
        os << (j == end ? "" : ":") << groups[j];
    return os.str();
}

IPv6Address::Scope IPv6Address::getScope() const
{
    //Mask the given IPv6 address with the different mask types
    //to get only the IPv6 address scope. Compare the masked
    //address with the different prefixes.

    if ((d[0] & LINK_LOCAL_MASK) == LINK_LOCAL_PREFIX) {
        return LINK;
    }
    else if ((d[0] & SITE_LOCAL_MASK) == SITE_LOCAL_PREFIX) {
        return SITE;
    }
    else if ((d[0] & MULTICAST_MASK) == MULTICAST_PREFIX) {
        return MULTICAST;
    }
    else if (d[0] == 0x00000000 && d[1] == 0x00000000 && d[2] == 0x00000000) {
        if (d[3] == 0x00000000) {
            return UNSPECIFIED;
        }
        else if (d[3] == 0x00000001) {
            return LOOPBACK;
        }
        else {
            return GLOBAL;    // actually an "IPv4-compatible IPv6 address"
        }
    }
    else {
        return GLOBAL;
    }
}

const char *IPv6Address::scopeName(Scope scope)
{
    switch (scope) {
        case UNSPECIFIED:
            return "unspec";

        case LOOPBACK:
            return "loopback";

        case MULTICAST:
            return "mcast";

        case LINK:
            return "link";

        case SITE:
            return "site";

        case GLOBAL:
            return "global";

        default:
            return "???";
    }
}

void IPv6Address::constructMask(int prefixLength, uint32 *mask)
{
    ASSERT(prefixLength >= 0 && prefixLength <= 128 && mask != nullptr);

    // create a mask based on the prefix length.
    if (prefixLength == 0) {
        mask[0] = mask[1] = mask[2] = mask[3] = 0x00000000;
    }
    else if (prefixLength <= 32) {
        int num_of_shifts = 32 - prefixLength;
        mask[0] = 0xFFFFFFFFU << num_of_shifts;
        mask[1] = 0x00000000;
        mask[2] = 0x00000000;
        mask[3] = 0x00000000;
    }
    else if (prefixLength <= 64) {
        int num_of_shifts = 64 - prefixLength;
        mask[0] = 0xFFFFFFFFU;
        mask[1] = 0xFFFFFFFFU << num_of_shifts;
        mask[2] = 0x00000000;
        mask[3] = 0x00000000;
    }
    else if (prefixLength <= 96) {
        int num_of_shifts = 96 - prefixLength;
        mask[0] = 0xFFFFFFFFU;
        mask[1] = 0xFFFFFFFFU;
        mask[2] = 0xFFFFFFFFU << num_of_shifts;
        mask[3] = 0x00000000;
    }
    else {
        int num_of_shifts = 128 - prefixLength;
        mask[0] = 0xFFFFFFFFU;
        mask[1] = 0xFFFFFFFFU;
        mask[2] = 0xFFFFFFFFU;
        mask[3] = 0xFFFFFFFFU << num_of_shifts;
    }
}

IPv6Address IPv6Address::constructMask(int prefixLength)
{
    IPv6Address ret;
    constructMask(prefixLength, ret.d);
    return ret;
}

IPv6Address IPv6Address::getPrefix(int prefixLength) const
{
    // First we construct a mask.
    uint32 mask[4];
    constructMask(prefixLength, mask);

    // Now we mask each IPv6 address segment and create a new IPv6 Address!
    return IPv6Address(d[0] & mask[0], d[1] & mask[1], d[2] & mask[2], d[3] & mask[3]);
}

IPv6Address IPv6Address::getSuffix(int prefixLength) const
{
    // First we construct a mask.
    uint32 mask[4];
    constructMask(prefixLength, mask);

    // Now we mask each IPv6 address segment, inverse it
    // and create a new IPv6 Address!
    return IPv6Address(d[0] & ~mask[0], d[1] & ~mask[1], d[2] & ~mask[2], d[3] & ~mask[3]);
}

const IPv6Address& IPv6Address::setPrefix(const IPv6Address& fromAddr, int prefixLength)
{
    // first we construct a mask.
    uint32 mask[4];
    constructMask(prefixLength, mask);

    // combine the addresses
    d[0] = (d[0] & ~mask[0]) | (fromAddr.d[0] & mask[0]);
    d[1] = (d[1] & ~mask[1]) | (fromAddr.d[1] & mask[1]);
    d[2] = (d[2] & ~mask[2]) | (fromAddr.d[2] & mask[2]);
    d[3] = (d[3] & ~mask[3]) | (fromAddr.d[3] & mask[3]);
    return *this;
}

const IPv6Address& IPv6Address::setSuffix(const IPv6Address& fromAddr, int prefixLength)
{
    // first we construct a mask.
    uint32 mask[4];
    constructMask(prefixLength, mask);

    // combine the addresses
    d[0] = (d[0] & mask[0]) | (fromAddr.d[0] & ~mask[0]);
    d[1] = (d[1] & mask[1]) | (fromAddr.d[1] & ~mask[1]);
    d[2] = (d[2] & mask[2]) | (fromAddr.d[2] & ~mask[2]);
    d[3] = (d[3] & mask[3]) | (fromAddr.d[3] & ~mask[3]);
    return *this;
}

IPv6Address IPv6Address::formLinkLocalAddress(const InterfaceToken& ident)
{
    IPv6Address suffix(0, 0, ident.normal(), ident.low());
    IPv6Address linkLocalAddr = IPv6Address::LINKLOCAL_PREFIX;
    linkLocalAddr.setSuffix(suffix, 128 - ident.length());
    return linkLocalAddr;
}

bool IPv6Address::matches(const IPv6Address& prefix, int prefixLength) const
{
    // first we construct a mask.
    uint32 mask[4];
    constructMask(prefixLength, mask);

    // xor the bits of the 2 addresses, and the result should be zero wherever
    // the mask has 1 bits
    return (((d[0] ^ prefix.d[0]) & mask[0]) | ((d[1] ^ prefix.d[1]) & mask[1])
            | ((d[2] ^ prefix.d[2]) & mask[2]) | ((d[3] ^ prefix.d[3]) & mask[3])) == 0;
}

int IPv6Address::getMulticastScope() const
{
    if ((d[0] & MULTICAST_MASK) != MULTICAST_PREFIX)
        throw cRuntimeError("IPv6Address::getMulticastScope(): %s is not a multicast address", str().c_str());
    return (d[0] >> 16) & 0x0F;
}

} // namespace inet

