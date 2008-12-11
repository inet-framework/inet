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

#include "IPAddress.h"

/**
 * Buffer length needed to hold an IP address in string form (dotted decimal notation)
 */
static const int IPADDRESS_STRING_SIZE = 20;

// predefined addresses
const IPAddress IPAddress::UNSPECIFIED_ADDRESS;
const IPAddress IPAddress::LOOPBACK_ADDRESS("127.0.0.1");
const IPAddress IPAddress::LOOPBACK_NETMASK("255.0.0.0");
const IPAddress IPAddress::ALLONES_ADDRESS("255.255.255.255");

const IPAddress IPAddress::ALL_HOSTS_MCAST("224.0.0.1");
const IPAddress IPAddress::ALL_ROUTERS_MCAST("224.0.0.2");
const IPAddress IPAddress::ALL_DVMRP_ROUTERS_MCAST("224.0.0.4");
const IPAddress IPAddress::ALL_OSPF_ROUTERS_MCAST("224.0.0.5");
const IPAddress IPAddress::ALL_OSPF_DESIGNATED_ROUTERS_MCAST("224.0.0.6");


void IPAddress::set(int i0, int i1, int i2, int i3)
{
    addr = (i0 << 24) | (i1 << 16) | (i2 << 8) | i3;
}

bool IPAddress::parseIPAddress(const char *text, unsigned char tobytes[])
{
    if (!text)
        return false;

    if (!strcmp(text,"<unspec>"))
    {
        tobytes[0] = tobytes[1] = tobytes[2] = tobytes[3] = 0;
        return true;
    }

    const char *s = text;
    int i=0;
    while(true)
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

void IPAddress::set(const char *text)
{
    unsigned char buf[4];
    if (!text)
        opp_error("IP address string is NULL");
    bool ok = parseIPAddress(text, buf);
    if (!ok)
        opp_error("Invalid IP address string `%s'", text);
    set(buf[0], buf[1], buf[2], buf[3]);
}

std::string IPAddress::str() const
{
    if (isUnspecified())
        return std::string("<unspec>");

    char buf[IPADDRESS_STRING_SIZE];
    sprintf(buf, "%u.%u.%u.%u", (addr>>24)&255, (addr>>16)&255, (addr>>8)&255, addr&255);
    return std::string(buf);
}

char IPAddress::getIPClass() const
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

IPAddress IPAddress::getNetwork() const
{
    switch (getIPClass())
    {
    case 'A':
        // Class A: network = 7 bits
        return IPAddress(getDByte(0), 0, 0, 0);
    case 'B':
        // Class B: network = 14 bits
        return IPAddress(getDByte(0), getDByte(1), 0, 0);
    case 'C':
        // Class C: network = 21 bits
        return IPAddress(getDByte(0), getDByte(1), getDByte(2), 0);
    default:
        // Class D or E
        return IPAddress();
    }
}

IPAddress IPAddress::getNetworkMask() const
{
    switch (getIPClass())
    {
    case 'A':
        // Class A: network = 7 bits
        return IPAddress(255, 0, 0, 0);
    case 'B':
        // Class B: network = 14 bits
        return IPAddress(255, 255, 0, 0);
    case 'C':
        // Class C: network = 21 bits
        return IPAddress(255, 255, 255, 0);
    default:
        // Class D or E: return null address
        return IPAddress();
    }
}


bool IPAddress::isNetwork(const IPAddress& toCmp) const
{
    IPAddress netmask = getNetworkMask();
    if (netmask.isUnspecified()) return false; // Class is D or E
    return maskedAddrAreEqual(*this, toCmp, netmask);
}


bool IPAddress::prefixMatches(const IPAddress& to_cmp, int numbits) const
{
    if (numbits<1)
        return true;

    uint32 addr2 = to_cmp.getInt();

    if (numbits > 31)
        return addr==addr2;

    // The right shift on an unsigned int produces 0 on the left
    uint32 mask = 0xFFFFFFFF;
    mask = ~(mask >> numbits);

    return (addr & mask) == (addr2 & mask);
}

int IPAddress::getNumMatchingPrefixBits(const IPAddress& to_cmp) const
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

int IPAddress::getNetmaskLength() const
{
    int i;
    for (i=0; i<31; i++)
        if (addr & (1 << i))
            return 32-i;
    return 0;
}

void IPAddress::keepFirstBits(unsigned int n)
{
    addr &= 0xFFFFFFFF << n;
}

bool IPAddress::maskedAddrAreEqual(const IPAddress& addr1,
                                   const IPAddress& addr2,
                                   const IPAddress& netmask)
{
    // return addr1.doAnd(netmask).equals(addr2.doAnd(netmask));
    // Looks weird, but is the same and is faster
    return !(bool)((addr1.addr ^ addr2.addr) & netmask.addr);
}

bool IPAddress::isWellFormed(const char *text)
{
    unsigned char dummy[4];
    return parseIPAddress(text, dummy);
}


