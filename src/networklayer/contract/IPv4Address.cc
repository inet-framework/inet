//
// Copyright (C) 2001  Vincent Oberle (vincent@oberle.com)
// Institute of Telematics, University of Karlsruhe, Germany.
// University Comillas, Madrid, Spain.
// Copyright (C) 2004, 2008 Andras Varga
// Copyright (C) 2008  Ingmar Baumgart
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


//
//  Author: Vincent Oberle
//  Date: Jan-March 2001
//  Cleanup and rewrite: Andras Varga, 2004
//

#include "IPv4Address.h"

/**
 * Buffer length needed to hold an IPv4 address in string form (dotted decimal notation)
 */
static const int IPADDRESS_STRING_SIZE = 20;

// predefined addresses
const IPv4Address IPv4Address::UNSPECIFIED_ADDRESS;
const IPv4Address IPv4Address::LOOPBACK_ADDRESS("127.0.0.1");
const IPv4Address IPv4Address::LOOPBACK_NETMASK("255.0.0.0");
const IPv4Address IPv4Address::ALLONES_ADDRESS("255.255.255.255");

const IPv4Address IPv4Address::ALL_HOSTS_MCAST("224.0.0.1");
const IPv4Address IPv4Address::ALL_ROUTERS_MCAST("224.0.0.2");
const IPv4Address IPv4Address::ALL_DVMRP_ROUTERS_MCAST("224.0.0.4");
const IPv4Address IPv4Address::ALL_OSPF_ROUTERS_MCAST("224.0.0.5");
const IPv4Address IPv4Address::ALL_OSPF_DESIGNATED_ROUTERS_MCAST("224.0.0.6");

const IPv4Address IPv4Address::LL_MANET_ROUTERS("224.0.0.109");

void IPv4Address::set(int i0, int i1, int i2, int i3)
{
    addr = (i0 << 24) | (i1 << 16) | (i2 << 8) | i3;
}

bool IPv4Address::parseIPAddress(const char *text, unsigned char tobytes[])
{
    if (!text)
        return false;

    if (!strcmp(text, "<unspec>"))
    {
        tobytes[0] = tobytes[1] = tobytes[2] = tobytes[3] = 0;
        return true;
    }

    const char *s = text;
    int i = 0;
    while (true)
    {
        if (*s<'0' || *s>'9')
            return false;  // missing number

        // read and store number
        int num = 0;
        while (*s>='0' && *s<='9')
            num = 10*num + (*s++ - '0');
        if (num>255)
            return false; // number too big
        tobytes[i++] = (unsigned char) num;

        if (!*s)
            break;  // end of string
        if (*s!='.')
            return false;  // invalid char after number
        if (i==4)
            return false;  // read 4th number and not yet EOS

        // skip '.'
        s++;
    }
    return i==4;  // must have all 4 numbers
}

void IPv4Address::set(const char *text)
{
    unsigned char buf[4];
    if (!text)
        throw cRuntimeError("IPv4 address string is NULL");

    bool ok = parseIPAddress(text, buf);
    if (!ok)
        throw cRuntimeError("Invalid IPv4 address string `%s'", text);

    set(buf[0], buf[1], buf[2], buf[3]);
}

std::string IPv4Address::str(bool printUnspec /* = true */) const
{
    if (printUnspec && isUnspecified())
        return std::string("<unspec>");

    char buf[IPADDRESS_STRING_SIZE];
    sprintf(buf, "%u.%u.%u.%u", (addr>>24)&255, (addr>>16)&255, (addr>>8)&255, addr&255);
    return std::string(buf);
}

char IPv4Address::getIPClass() const
{
    unsigned char buf = getDByte(0);
    if ((buf & 0x80) == 0x00)       // 0xxxx
        return 'A';
    else if ((buf & 0xC0) == 0x80)  // 10xxx
        return 'B';
    else if ((buf & 0xE0) == 0xC0)  // 110xx
        return 'C';
    else if ((buf & 0xF0) == 0xE0)  // 1110x
        return 'D';
    else if ((buf & 0xF8) == 0xF0)  // 11110
        return 'E';
    else
        return '?';
}

IPv4Address::AddressCategory IPv4Address::getAddressCategory() const
{
    if (isUnspecified())
        return UNSPECIFIED;        // 0.0.0.0
    if ((addr & 0xFF000000u) == 0)
        return THIS_NETWORK;       // 0.0.0.0/8
    if ((addr & 0xFF000000u) == 0x7F000000u)
        return LOOPBACK;           // 127.0.0.0/8
    if (isMulticast())
        return MULTICAST;          // 224.0.0.0/4
    if (addr == 0xFFFFFFFFu)
        return BROADCAST;          // 255.255.255.255/32
    uint32 addr24 = addr & 0xFFFFFF00u;
    if (addr24 == 0xC0000000u)
        return IETF;               // 192.0.0.0/24
    if ((addr24 == 0xC0000200u) || (addr24 == 0xC6336400u) || (addr24 == 0xCB007100u))
        return TEST_NET;           // 192.0.2.0/24, 198.51.100.0/24, 203.0.113.0/24
    if (addr24 == 0xC0586300u)
        return IPv6_TO_IPv4_RELAY; // 192.88.99.0/24
    if ((addr & 0xFFFE0000u) == 0xC6120000u)
        return BENCHMARK;          // 198.18.0.0/15
    if ((addr & 0xF0000000u) == 0xF0000000u)
        return RESERVED;           // 240.0.0.0/4
    if ((addr & 0xFFFF0000u) == 0xA9FE0000u)
        return LINKLOCAL;          // 169.254.0.0/16
    if (((addr & 0xFF000000u) == 0x0A000000u)
            || ((addr & 0xFFF00000u) == 0xAC100000u)
            || ((addr & 0xFFFF0000u) == 0xC0A80000u))
        return PRIVATE_NETWORK;    // 10.0.0.0/8, 172.16.0.0/12, 192.168.0.0/16
    return GLOBAL;
}

IPv4Address IPv4Address::getNetwork() const
{
    switch (getIPClass())
    {
    case 'A':
        // Class A: network = 7 bits
        return IPv4Address(getDByte(0), 0, 0, 0);
    case 'B':
        // Class B: network = 14 bits
        return IPv4Address(getDByte(0), getDByte(1), 0, 0);
    case 'C':
        // Class C: network = 21 bits
        return IPv4Address(getDByte(0), getDByte(1), getDByte(2), 0);
    default:
        // Class D or E
        return IPv4Address();
    }
}

IPv4Address IPv4Address::getNetworkMask() const
{
    switch (getIPClass())
    {
    case 'A':
        // Class A: network = 7 bits
        return IPv4Address(255, 0, 0, 0);
    case 'B':
        // Class B: network = 14 bits
        return IPv4Address(255, 255, 0, 0);
    case 'C':
        // Class C: network = 21 bits
        return IPv4Address(255, 255, 255, 0);
    default:
        // Class D or E: return null address
        return IPv4Address();
    }
}


bool IPv4Address::isNetwork(const IPv4Address& toCmp) const
{
    IPv4Address netmask = getNetworkMask();
    if (netmask.isUnspecified()) return false; // Class is D or E
    return maskedAddrAreEqual(*this, toCmp, netmask);
}


bool IPv4Address::prefixMatches(const IPv4Address& other, int length) const
{
    if (length<1)
        return true;
    if (length > 31)
        return addr==other.addr;

    uint32 mask = _makeNetmask(length);
    return (addr & mask) == (other.addr & mask);
}

int IPv4Address::getNumMatchingPrefixBits(const IPv4Address& to_cmp) const
{
    uint32 addr2 = to_cmp.getInt();

    uint32 res = addr ^ addr2;
    // If the bits are equal, there is a 0, so counting
    // the zeros from the left
    for (int i = 31; i >= 0; i--) {
        if (res & (1 << i)) {
            // 1, means not equal, so stop
            return 31 - i;
        }
    }
    return 32;
}

int IPv4Address::getNetmaskLength() const
{
    for (int i=0; i<32; i++)
        if (addr & (1 << i))
            return 32-i;
    return 0;
}

void IPv4Address::_checkNetmaskLength(int length)
{
    if (length < 0 || length > 32)
        throw cRuntimeError("IPv4Address: wrong netmask length %d (not in 0..32)", length);
}

bool IPv4Address::maskedAddrAreEqual(const IPv4Address& addr1,
                                   const IPv4Address& addr2,
                                   const IPv4Address& netmask)
{
    // return addr1.doAnd(netmask).equals(addr2.doAnd(netmask));
    // Looks weird, but is the same and is faster
    return !(bool)((addr1.addr ^ addr2.addr) & netmask.addr);
}

bool IPv4Address::isWellFormed(const char *text)
{
    unsigned char dummy[4];
    return parseIPAddress(text, dummy);
}


IPv4Address IPv4Address::getBroadcastAddress(IPv4Address netmask)
{
   IPv4Address br(getInt() | ~(netmask.getInt()));
   return br;
}

