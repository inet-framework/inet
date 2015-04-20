/*
 * Copyright (C) 2003 Andras Varga; CTIE, Monash University, Australia
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include <ctype.h>
#include "inet/linklayer/common/MACAddress.h"
#include "inet/networklayer/common/InterfaceToken.h"
#include "inet/networklayer/contract/ipv4/IPv4Address.h"

namespace inet {

unsigned int MACAddress::autoAddressCtr;
bool MACAddress::simulationLifecycleListenerAdded;

const MACAddress MACAddress::UNSPECIFIED_ADDRESS;
const MACAddress MACAddress::BROADCAST_ADDRESS("ff:ff:ff:ff:ff:ff");
const MACAddress MACAddress::MULTICAST_PAUSE_ADDRESS("01:80:C2:00:00:01");
const MACAddress MACAddress::STP_MULTICAST_ADDRESS("01:80:C2:00:00:00");

unsigned char MACAddress::getAddressByte(unsigned int k) const
{
    if (k >= MAC_ADDRESS_SIZE)
        throw cRuntimeError("Array of size 6 indexed with %d", k);
    int offset = (MAC_ADDRESS_SIZE - k - 1) * 8;
    return 0xff & (address >> offset);
}

void MACAddress::setAddressByte(unsigned int k, unsigned char addrbyte)
{
    if (k >= MAC_ADDRESS_SIZE)
        throw cRuntimeError("Array of size 6 indexed with %d", k);
    int offset = (MAC_ADDRESS_SIZE - k - 1) * 8;
    address = (address & (~(((uint64)0xff) << offset))) | (((uint64)addrbyte) << offset);
}

bool MACAddress::tryParse(const char *hexstr)
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
    address = 0;    // clear top 16 bits too that setAddressByte() calls skip
    int k = 0;
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
            k++;
        }
    }
    return true;
}

void MACAddress::setAddress(const char *hexstr)
{
    if (!tryParse(hexstr))
        throw cRuntimeError("MACAddress: wrong address syntax '%s': 12 hex digits expected, with optional embedded spaces, hyphens or colons", hexstr);
}

void MACAddress::getAddressBytes(unsigned char *addrbytes) const
{
    for (int i = 0; i < MAC_ADDRESS_SIZE; i++)
        addrbytes[i] = getAddressByte(i);
}

void MACAddress::setAddressBytes(unsigned char *addrbytes)
{
    address = 0;    // clear top 16 bits too that setAddressByte() calls skip
    for (int i = 0; i < MAC_ADDRESS_SIZE; i++)
        setAddressByte(i, addrbytes[i]);
}

std::string MACAddress::str() const
{
    char buf[20];
    char *s = buf;
    for (int i = 0; i < MAC_ADDRESS_SIZE; i++, s += 3)
        sprintf(s, "%2.2X-", getAddressByte(i));
    *(s - 1) = '\0';
    return std::string(buf);
}

int MACAddress::compareTo(const MACAddress& other) const
{
    return (address < other.address) ? -1 : (address == other.address) ? 0 : 1;    // note: "return address-other.address" is not OK because 64-bit result does not fit into the return type
}

InterfaceToken MACAddress::formInterfaceIdentifier() const
{
    uint32 high = ((address >> 16) | 0xff) ^ 0x02000000;
    uint32 low = (0xfe << 24) | (address & 0xffffff);
    return InterfaceToken(low, high, 64);
}

MACAddress MACAddress::generateAutoAddress()
{
#if OMNETPP_VERSION >= 0x500
    if (!simulationLifecycleListenerAdded) {
        // NOTE: EXECUTE_ON_STARTUP is too early and would add the listener to StaticEnv
        getEnvir()->addLifecycleListener(new MACAddress::SimulationLifecycleListener());
        simulationLifecycleListenerAdded = true;
    }
#endif // if OMNETPP_VERSION >= 0x500
    ++autoAddressCtr;

    uint64 intAddr = 0x0AAA00000000ULL + (autoAddressCtr & 0xffffffffUL);
    MACAddress addr(intAddr);
    return addr;
}

// see  RFC 1112, section 6.4
MACAddress MACAddress::makeMulticastAddress(IPv4Address addr)
{
    ASSERT(addr.isMulticast());

    MACAddress macAddr;
    macAddr.setAddressByte(0, 0x01);
    macAddr.setAddressByte(1, 0x00);
    macAddr.setAddressByte(2, 0x5e);
    macAddr.setAddressByte(3, addr.getDByte(1) & 0x7f);
    macAddr.setAddressByte(4, addr.getDByte(2));
    macAddr.setAddressByte(5, addr.getDByte(3));
    return macAddr;
}

} // namespace inet

