//
// Copyright (C) 2003 Andras Varga; CTIE, Monash University, Australia
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/linklayer/common/MacAddress.h"

#include <ctype.h>

#include "inet/networklayer/common/InterfaceToken.h"

namespace inet {

const MacAddress MacAddress::UNSPECIFIED_ADDRESS;
const MacAddress MacAddress::BROADCAST_ADDRESS("ff:ff:ff:ff:ff:ff");
const MacAddress MacAddress::MULTICAST_PAUSE_ADDRESS("01:80:C2:00:00:01");
const MacAddress MacAddress::STP_MULTICAST_ADDRESS("01:80:C2:00:00:00");

const MacAddress MacAddress::CDP_MULTICAST_ADDRESS("01:00:0C:CC:CC:CC");
const MacAddress MacAddress::LLDP_MULTICAST_ADDRESS("01:80:C2:00:00:0E");

unsigned char MacAddress::getAddressByte(unsigned int k) const
{
    if (k >= MAC_ADDRESS_SIZE)
        throw cRuntimeError("Array of size 6 indexed with %d", k);
    int offset = (MAC_ADDRESS_SIZE - k - 1) * 8;
    return 0xff & (address >> offset);
}

void MacAddress::setAddressByte(unsigned int k, unsigned char addrbyte)
{
    if (k >= MAC_ADDRESS_SIZE)
        throw cRuntimeError("Array of size 6 indexed with %d", k);
    int offset = (MAC_ADDRESS_SIZE - k - 1) * 8;
    address = (address & (~(((uint64_t)0xff) << offset))) | (((uint64_t)addrbyte) << offset);
}

bool MacAddress::tryParse(const char *hexstr)
{
    if (!hexstr)
        return false;

    // check syntax
    int numHexDigits = 0;
    for (const char *s = hexstr; *s; s++) {
        if (isxdigit(*s))
            numHexDigits++;
        else if (*s != ' ' && *s != ':' && *s != '-')
            return false; // wrong syntax
    }
    if (numHexDigits != 2 * MAC_ADDRESS_SIZE)
        return false;

    // Converts hex string into the address
    // if hext string is shorter, address is filled with zeros;
    // Non-hex characters are discarded before conversion.
    address = 0; // clear top 16 bits too that setAddressByte() calls skip
    const char *s = hexstr;
    for (int pos = 0; pos < MAC_ADDRESS_SIZE; pos++) {
        if (!s || !*s) {
            setAddressByte(pos, 0);
        }
        else {
            while (*s && !isxdigit(*s))
                s++;
            if (!*s) {
                setAddressByte(pos, 0);
                continue;
            }
            unsigned char d = isdigit(*s) ? (*s - '0') : islower(*s) ? (*s - 'a' + 10) : (*s - 'A' + 10);
            d = d << 4;
            s++;

            while (*s && !isxdigit(*s))
                s++;
            if (!*s) {
                setAddressByte(pos, 0);
                continue;
            }
            d += isdigit(*s) ? (*s - '0') : islower(*s) ? (*s - 'a' + 10) : (*s - 'A' + 10);
            s++;

            setAddressByte(pos, d);
        }
    }
    return true;
}

void MacAddress::setAddress(const char *hexstr)
{
    if (!tryParse(hexstr))
        throw cRuntimeError("MacAddress: wrong address syntax '%s': 12 hex digits expected, with optional embedded spaces, hyphens or colons", hexstr);
}

void MacAddress::getAddressBytes(unsigned char *addrbytes) const
{
    for (int i = 0; i < MAC_ADDRESS_SIZE; i++)
        addrbytes[i] = getAddressByte(i);
}

void MacAddress::setAddressBytes(unsigned char *addrbytes)
{
    address = 0; // clear top 16 bits too that setAddressByte() calls skip
    for (int i = 0; i < MAC_ADDRESS_SIZE; i++)
        setAddressByte(i, addrbytes[i]);
}

std::string MacAddress::str() const
{
    char buf[20];
    char *s = buf;
    for (int i = 0; i < MAC_ADDRESS_SIZE; i++, s += 3)
        sprintf(s, "%2.2X-", getAddressByte(i));
    *(s - 1) = '\0';
    return std::string(buf);
}

int MacAddress::compareTo(const MacAddress& other) const
{
    return (address < other.address) ? -1 : (address == other.address) ? 0 : 1; // note: "return address-other.address" is not OK because 64-bit result does not fit into the return type
}

InterfaceToken MacAddress::formInterfaceIdentifier() const
{
    uint32_t high = ((address >> 16) | 0xff) ^ 0x02000000;
    uint32_t low = (0xfe << 24) | (address & 0xffffff);
    return InterfaceToken(low, high, 64);
}

MacAddress MacAddress::generateAutoAddress()
{
    static int handle = cSimulationOrSharedDataManager::registerSharedCounterName("inet::MacAddress::counter");
    uint64_t& counter = getSimulationOrSharedDataManager()->getSharedCounter(handle, getActiveSimulationOrEnvir()->getParsimProcId() * 0x000100000000ULL);
    uint64_t raw = 0x0AAA00000000ULL + (++counter & 0xffffffffUL);
    return MacAddress(raw);
}

} // namespace inet
