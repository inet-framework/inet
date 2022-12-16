//
// Copyright (C) 2003 Andras Varga; CTIE, Monash University, Australia
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_MACADDRESS_H
#define __INET_MACADDRESS_H

#include <string>

#include "inet/common/INETDefs.h"

namespace inet {

#define MAC_ADDRESS_SIZE    6
#define MAC_ADDRESS_MASK    0xffffffffffffULL

class InterfaceToken;

/**
 * Stores an IEEE 802 MAC address (6 octets = 48 bits).
 */
class INET_API MacAddress
{
  private:
    uint64_t address; // 6*8=48 bit address, lowest 6 bytes are used, highest 2 bytes are always zero

  public:
    /** The unspecified MAC address, 00:00:00:00:00:00 */
    static const MacAddress UNSPECIFIED_ADDRESS;

    /** The broadcast MAC address, ff:ff:ff:ff:ff:ff */
    static const MacAddress BROADCAST_ADDRESS;

    /** The special multicast PAUSE MAC address, 01:80:C2:00:00:01 */
    static const MacAddress MULTICAST_PAUSE_ADDRESS;

    /** The spanning tree protocol bridge's multicast address, 01:80:C2:00:00:00 */
    static const MacAddress STP_MULTICAST_ADDRESS;

    /** The Cisco discovery protocol bridge's multicast address, 01:00:0C:CC:CC:CC */
    static const MacAddress CDP_MULTICAST_ADDRESS;

    /** The Local link discovery protocol bridge's multicast address, 01:80:C2:00:00:0E */
    static const MacAddress LLDP_MULTICAST_ADDRESS;

    /**
     * Default constructor initializes address bytes to zero.
     */
    MacAddress() { address = 0; }

    /**
     * Initializes the address from the lower 48 bits of the 64-bit argument
     */
    explicit MacAddress(uint64_t bits) { address = bits & MAC_ADDRESS_MASK; }

    /**
     * Constructor which accepts a hex string (12 hex digits, may also
     * contain spaces, hyphens and colons)
     */
    explicit MacAddress(const char *hexstr) { setAddress(hexstr); }

    /**
     * Copy constructor.
     */
    MacAddress(const MacAddress& other) { address = other.address; }

    /**
     * Assignment.
     */
    MacAddress& operator=(const MacAddress& other) { address = other.address; return *this; }

    /**
     * Returns the address size in bytes, that is, 6.
     */
    unsigned int getAddressSize() const { return MAC_ADDRESS_SIZE; }

    /**
     * Returns the kth byte of the address.
     */
    unsigned char getAddressByte(unsigned int k) const;

    /**
     * Sets the kth byte of the address.
     */
    void setAddressByte(unsigned int k, unsigned char addrbyte);

    /**
     * Sets the address and returns true if the syntax of the string
     * is correct. (See setAddress() for the syntax.)
     */
    bool tryParse(const char *hexstr);

    /**
     * Converts address value from hex string (12 hex digits, may also
     * contain spaces, hyphens and colons)
     */
    void setAddress(const char *hexstr);

    /**
     * Copies the address to the given pointer (array of 6 unsigned chars).
     */
    void getAddressBytes(unsigned char *addrbytes) const;
    void getAddressBytes(char *addrbytes) const { getAddressBytes(reinterpret_cast<unsigned char *>(addrbytes)); }

    /**
     * Sets address bytes. The argument should point to an array of 6 unsigned chars.
     */
    void setAddressBytes(unsigned char *addrbytes);
    void setAddressBytes(char *addrbytes) { setAddressBytes(reinterpret_cast<unsigned char *>(addrbytes)); }

    /**
     * Sets the address to the broadcast address (hex ff:ff:ff:ff:ff:ff).
     */
    void setBroadcast() { address = MAC_ADDRESS_MASK; }

    /**
     * Returns true if this is the broadcast address (hex ff:ff:ff:ff:ff:ff).
     */
    bool isBroadcast() const { return address == MAC_ADDRESS_MASK; }

    /**
     * Returns true if this is a multicast logical address (first byte's lsb is 1).
     */
    bool isMulticast() const { return getAddressByte(0) & 0x01; };

    /**
     * Returns true if this is a local address (first byte's second less significant bit is 1).
     */
    bool isLocal() const { return getAddressByte(0) & 0x02; };

    /**
     * Returns true if all address bytes are zero.
     */
    bool isUnspecified() const { return address == 0; }

    /**
     * Converts address to a hex string.
     */
    std::string str() const;

    /**
     * Converts address to 48 bits integer.
     */
    uint64_t getInt() const { return address; }

    /**
     * Returns true if the two addresses are equal.
     */
    bool equals(const MacAddress& other) const { return address == other.address; }

    /**
     * Returns true if the two addresses are equal.
     */
    bool operator==(const MacAddress& other) const { return address == other.address; }

    /**
     * Returns true if the two addresses are not equal.
     */
    bool operator!=(const MacAddress& other) const { return address != other.address; }

    /**
     * Returns -1, 0 or 1 as result of comparison of 2 addresses.
     */
    int compareTo(const MacAddress& other) const;

    /**
     * Create interface identifier (IEEE EUI-64) which can be used by IPv6
     * stateless address autoconfiguration.
     */
    InterfaceToken formInterfaceIdentifier() const;

    /**
     * Generates a unique address which begins with 0a:aa and ends in a unique
     * suffix.
     */
    static MacAddress generateAutoAddress();

    bool operator<(const MacAddress& other) const { return address < other.address; }

    bool operator>(const MacAddress& other) const { return address > other.address; }
};

inline std::ostream& operator<<(std::ostream& os, const MacAddress& mac)
{
    return os << mac.str();
}

inline void doParsimPacking(cCommBuffer *buffer, const MacAddress& macAddress) { buffer->pack(macAddress.getInt()); }
inline void doParsimUnpacking(cCommBuffer *buffer, MacAddress& macAddress) { uint64_t address; buffer->unpack(address); macAddress = MacAddress(address); }

} // namespace inet

#endif

