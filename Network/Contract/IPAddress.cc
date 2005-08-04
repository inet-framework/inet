//
// Copyright (C) 2001  Vincent Oberle (vincent@oberle.com)
// Institute of Telematics, University of Karlsruhe, Germany.
// University Comillas, Madrid, Spain.
// Copyright (C) 2004 Andras Varga
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
// License along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//


//
//  Author: Vincent Oberle
//  Date: Jan-March 2001
//  Cleanup and rewrite: Andras Varga, 2004
//

#include "IPAddress.h"


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


IPAddress::IPAddress(uint32 ip)
{
    set(ip);
}

IPAddress::IPAddress(int i0, int i1, int i2, int i3)
{
    addr[0] = i0;
    addr[1] = i1;
    addr[2] = i2;
    addr[3] = i3;
}

IPAddress::IPAddress(const char *text)
{
    set(text);
}

IPAddress::IPAddress(const IPAddress& obj)
{
    operator=(obj);
}

void IPAddress::set(uint32 ip)
{
    addr[0] = (ip >> 24) & 0xFF;
    addr[1] = (ip >> 16) & 0xFF;
    addr[2] = (ip >> 8) & 0xFF;
    addr[3] = ip & 0xFF;
}

void IPAddress::set(int i0, int i1, int i2, int i3)
{
    addr[0] = i0;
    addr[1] = i1;
    addr[2] = i2;
    addr[3] = i3;
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
    if (!text)
        opp_error("IP address string is NULL");
    bool ok = parseIPAddress(text, addr);
    if (!ok)
        opp_error("Invalid IP address string `%s'", text);
}

uint32 IPAddress::getInt () const
{
    return (addr[0] << 24)
        +  (addr[1] << 16)
        +  (addr[2] << 8)
        +  (addr[3]);
}

IPAddress& IPAddress::operator=(const IPAddress& obj)
{
    addr[0] = obj.addr[0];
    addr[1] = obj.addr[1];
    addr[2] = obj.addr[2];
    addr[3] = obj.addr[3];
    return *this;
}

std::string IPAddress::str() const
{
    if (isUnspecified())
        return std::string("<unspec>");

    char buf[ADDRESS_STRING_SIZE];
    sprintf(buf, "%d.%d.%d.%d", addr[0], addr[1], addr[2], addr[3]);
    return std::string(buf);
}

bool IPAddress::equals(const IPAddress& toCmp) const
{
    return (addr[0] == toCmp.addr[0]) && (addr[1] == toCmp.addr[1]) &&
           (addr[2] == toCmp.addr[2]) && (addr[3] == toCmp.addr[3]);
}

IPAddress IPAddress::doAnd(const IPAddress& ip) const
{
    return IPAddress(addr[0] & ip.addr[0], addr[1] & ip.addr[1],
                     addr[2] & ip.addr[2], addr[3] & ip.addr[3]);
}


char IPAddress::getIPClass() const
{
    if ((addr[0] & 0x80) == 0x00)       // 0xxxx
        return 'A';
    else if ((addr[0] & 0xC0) == 0x80)  // 10xxx
        return 'B';
    else if ((addr[0] & 0xE0) == 0xC0)  // 110xx
        return 'C';
    else if ((addr[0] & 0xF0) == 0xE0)  // 1110x
        return 'D';
    else if ((addr[0] & 0xF8) == 0xF0)  // 11110
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
        return IPAddress(addr[0], 0, 0, 0);
    case 'B':
        // Class B: network = 14 bits
        return IPAddress(addr[0], addr[1], 0, 0);
    case 'C':
        // Class C: network = 21 bits
        return IPAddress(addr[0], addr[1], addr[2], 0);
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
    switch (getIPClass())
    {
    case 'A':
        if (addr[0] == toCmp.addr[0])
            return true;
        break;
    case 'B':
        if ((addr[0] == toCmp.addr[0]) &&
            (addr[1] == toCmp.addr[1]))
            return true;
        break;
    case 'C':
        if ((addr[0] == toCmp.addr[0]) &&
            (addr[1] == toCmp.addr[1]) &&
            (addr[2] == toCmp.addr[2]))
            return true;
        break;
    default:
        // Class D or E
        return false;
    }
    // not equal
    return false;
}


bool IPAddress::prefixMatches(const IPAddress& to_cmp, int numbits) const
{
    if (numbits<1)
        return true;

    uint32 addr1 = getInt();
    uint32 addr2 = to_cmp.getInt();

    if (numbits > 31)
        return addr1==addr2;

    // The right shift on an unsigned int produces 0 on the left
    uint32 mask = 0xFFFFFFFF;
    mask = ~(mask >> numbits);

    return (addr1 & mask) == (addr2 & mask);
}

int IPAddress::numMatchingPrefixBits(const IPAddress& to_cmp) const
{
    uint32 addr1 = getInt();
    uint32 addr2 = to_cmp.getInt();

    uint32 res = addr1 ^ addr2;
    // If the bits are equal, there is a 0, so counting
    // the zeros from the left
    int i;
    for (i = 31; i >= 0; i--) {
        if (res & (1 << i)) {
            // 1, means not equal, so stop
            return 31 - i;
        }
    }
    return 32;
}

int IPAddress::netmaskLength() const
{
    uint32 addr = getInt();
    int i;
    for (i=0; i<31; i++)
        if (addr & (1 << i))
            return 32-i;
    return 0;
}

void IPAddress::keepFirstBits (unsigned int n)
{
    if (n > 31) return;

    int len_bytes = n / 8;

    uint32 mask = 0xFF;
    mask = ~(mask >> ((n - (len_bytes * 8))));

    addr[len_bytes] = addr[len_bytes] & mask;

    for (int i = len_bytes+1; i < 4; i++)
        addr[i] = 0;
}


bool IPAddress::maskedAddrAreEqual(const IPAddress& addr1,
                                   const IPAddress& addr2,
                                   const IPAddress& netmask)
{
    if (addr1.doAnd(netmask).equals(addr2.doAnd(netmask)))
        return true;

    return false;
}

bool IPAddress::isWellFormed(const char *text)
{
    unsigned char dummy[4];
    return parseIPAddress(text, dummy);
}


