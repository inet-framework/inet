// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_IEEE802154ADDRESS_H
#define __INET_IEEE802154ADDRESS_H

#include <cstdint>
#include <functional>
#include <ostream>
#include <string>

#include "inet/common/INETDefs.h"

namespace inet {

/**
 * A protocol-local IEEE 802.15.4 address value.
 *
 * The address type is part of the value. A short address and an extended
 * address with the same numerical value are therefore different values.
 * Extended values use the conventional numeric EUI-64 representation from
 * IEEE Std 802.15.4-2024, 7.1; wire octet order from 4.5.1 is a
 * responsibility of the frame codec.
 */
class INET_API Ieee802154Address
{
  public:
    enum class AddressType : uint8_t {
        NONE,
        SHORT,
        EXTENDED,
    };

    /** The absence of an IEEE 802.15.4 address. */
    static const Ieee802154Address NONE_ADDRESS;

    /** IEEE Std 802.15.4-2024, 10.21.5.2 and 10.4.12.2: associated without a short allocation. */
    static const Ieee802154Address SHORT_UNALLOCATED_ADDRESS;

    /** IEEE Std 802.15.4-2024, 6.2: the short broadcast address. */
    static const Ieee802154Address SHORT_BROADCAST_ADDRESS;

    /** IEEE Std 802.15.4-2024, 6.2 and IEEE Std 802-2024, 8.2.2: all-ones broadcast. */
    static const Ieee802154Address EXTENDED_BROADCAST_ADDRESS;

    /** IEEE Std 802-2024, 8.2.2: the numeric extended-address group bit (bit 56). */
    static constexpr uint64_t EXTENDED_GROUP_MASK = 0x0100000000000000ULL;

  private:
    AddressType type;
    uint64_t value;

    Ieee802154Address(AddressType type, uint64_t value) : type(type), value(value) {}

    /** Compares by AddressType, then by the complete numeric value. */
    int compareTo(const Ieee802154Address& other) const;

  public:
    /** Constructs the NONE value. */
    Ieee802154Address() : type(AddressType::NONE), value(0) {}

    /**
     * Parses one of exactly three textual forms: "<none>", "short:XXXX",
     * or "extended:XX-XX-XX-XX-XX-XX-XX-XX". Prefixes are lower-case,
     * separators are hyphens only, and hexadecimal digits are case-insensitive.
     */
    explicit Ieee802154Address(const char *text) { set(text); }

    /** Constructs a SHORT value, including the two protocol-defined sentinels. */
    static Ieee802154Address fromShort(uint16_t value) { return Ieee802154Address(AddressType::SHORT, value); }

    /** Constructs an EXTENDED value without narrowing or validating its bits. */
    static Ieee802154Address fromExtended(uint64_t value) { return Ieee802154Address(AddressType::EXTENDED, value); }

    AddressType getType() const { return type; }

    bool isNone() const { return type == AddressType::NONE; }
    bool isShort() const { return type == AddressType::SHORT; }
    bool isExtended() const { return type == AddressType::EXTENDED; }

    /** Returns true only for the 0xfffe short value. */
    bool isShortUnallocated() const { return isShort() && value == 0xfffe; }

    /** Returns true only for the 0xffff short value. */
    bool isShortBroadcast() const { return isShort() && value == 0xffff; }

    /** Returns true for either short or extended broadcast value. */
    bool isBroadcast() const { return isShortBroadcast() || (isExtended() && value == UINT64_MAX); }

    /** Returns true only for an EXTENDED value with the IEEE Std 802 group bit set. */
    bool isExtendedGroup() const { return isExtended() && (value & EXTENDED_GROUP_MASK) != 0; }

    /** Returns the short value, or throws if this value is not SHORT. */
    uint16_t getShort() const;

    /** Returns the extended value, or throws if this value is not EXTENDED. */
    uint64_t getExtended() const;

    /** Parses the three canonical forms and leaves this object unchanged on failure. */
    bool tryParse(const char *text);

    /** Parses a canonical value, throwing cRuntimeError on failure. */
    void set(const char *text);

    /** Returns the canonical textual representation. */
    std::string str() const;

    bool operator==(const Ieee802154Address& other) const { return type == other.type && value == other.value; }
    bool operator!=(const Ieee802154Address& other) const { return !(*this == other); }
    bool operator<(const Ieee802154Address& other) const { return compareTo(other) < 0; }
    bool operator>(const Ieee802154Address& other) const { return compareTo(other) > 0; }

    friend struct std::hash<Ieee802154Address>;
};

inline std::ostream& operator<<(std::ostream& os, const Ieee802154Address& address)
{
    return os << address.str();
}

} // namespace inet

namespace std {

template<> struct hash<inet::Ieee802154Address>
{
    size_t operator()(const inet::Ieee802154Address& address) const noexcept
    {
        uint64_t hashValue = address.value;
        hashValue ^= static_cast<uint64_t>(address.type) + 0x9e3779b97f4a7c15ULL + (hashValue << 6) + (hashValue >> 2);
        return std::hash<uint64_t>()(hashValue);
    }
};

} // namespace std

#endif
