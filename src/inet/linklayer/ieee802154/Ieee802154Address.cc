// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/linklayer/ieee802154/Ieee802154Address.h"

#include <cstring>

namespace inet {

const Ieee802154Address Ieee802154Address::NONE_ADDRESS;
const Ieee802154Address Ieee802154Address::SHORT_UNALLOCATED_ADDRESS = Ieee802154Address::fromShort(0xfffe);
const Ieee802154Address Ieee802154Address::SHORT_BROADCAST_ADDRESS = Ieee802154Address::fromShort(0xffff);
const Ieee802154Address Ieee802154Address::EXTENDED_BROADCAST_ADDRESS = Ieee802154Address::fromExtended(UINT64_MAX);

static int decodeHexDigit(char digit)
{
    if ('0' <= digit && digit <= '9')
        return digit - '0';
    if ('a' <= digit && digit <= 'f')
        return digit - 'a' + 10;
    if ('A' <= digit && digit <= 'F')
        return digit - 'A' + 10;
    return -1;
}

static bool decodeHexPair(const char *text, uint8_t& value)
{
    int high = decodeHexDigit(text[0]);
    int low = decodeHexDigit(text[1]);
    if (high < 0 || low < 0)
        return false;
    value = static_cast<uint8_t>((high << 4) | low);
    return true;
}

uint16_t Ieee802154Address::getShort() const
{
    if (!isShort())
        throw cRuntimeError("Ieee802154Address %s is not a short address", str().c_str());
    return static_cast<uint16_t>(value);
}

uint64_t Ieee802154Address::getExtended() const
{
    if (!isExtended())
        throw cRuntimeError("Ieee802154Address %s is not an extended address", str().c_str());
    return value;
}

bool Ieee802154Address::tryParse(const char *text)
{
    if (!text)
        return false;

    if (strcmp(text, "<none>") == 0) {
        *this = NONE_ADDRESS;
        return true;
    }

    constexpr size_t SHORT_PREFIX_LENGTH = 6; // "short:"
    if (strncmp(text, "short:", SHORT_PREFIX_LENGTH) == 0) {
        if (strlen(text) != SHORT_PREFIX_LENGTH + 4)
            return false;
        uint8_t high;
        uint8_t low;
        if (!decodeHexPair(text + SHORT_PREFIX_LENGTH, high) || !decodeHexPair(text + SHORT_PREFIX_LENGTH + 2, low))
            return false;
        *this = fromShort(static_cast<uint16_t>((high << 8) | low));
        return true;
    }

    constexpr size_t EXTENDED_PREFIX_LENGTH = 9; // "extended:"
    if (strncmp(text, "extended:", EXTENDED_PREFIX_LENGTH) == 0) {
        if (strlen(text) != EXTENDED_PREFIX_LENGTH + 23)
            return false;
        uint64_t extendedValue = 0;
        for (int i = 0; i < 8; i++) {
            size_t offset = EXTENDED_PREFIX_LENGTH + i * 3;
            if (i != 0 && text[offset - 1] != '-')
                return false;
            uint8_t octet;
            if (!decodeHexPair(text + offset, octet))
                return false;
            extendedValue = (extendedValue << 8) | octet;
        }
        *this = fromExtended(extendedValue);
        return true;
    }

    return false;
}

void Ieee802154Address::set(const char *text)
{
    if (!text)
        throw cRuntimeError("Ieee802154Address string is nullptr");
    if (!tryParse(text))
        throw cRuntimeError("Invalid Ieee802154Address string '%s'", text);
}

std::string Ieee802154Address::str() const
{
    if (isNone())
        return "<none>";

    static const char hexDigits[] = "0123456789ABCDEF";
    if (isShort()) {
        std::string result = "short:0000";
        result[6] = hexDigits[(value >> 12) & 0xf];
        result[7] = hexDigits[(value >> 8) & 0xf];
        result[8] = hexDigits[(value >> 4) & 0xf];
        result[9] = hexDigits[value & 0xf];
        return result;
    }

    std::string result = "extended:00-00-00-00-00-00-00-00";
    for (int i = 0; i < 8; i++) {
        uint8_t octet = static_cast<uint8_t>(value >> (56 - i * 8));
        size_t offset = 9 + i * 3;
        result[offset] = hexDigits[(octet >> 4) & 0xf];
        result[offset + 1] = hexDigits[octet & 0xf];
    }
    return result;
}

int Ieee802154Address::compareTo(const Ieee802154Address& other) const
{
    if (type != other.type)
        return static_cast<uint8_t>(type) < static_cast<uint8_t>(other.type) ? -1 : 1;
    return value < other.value ? -1 : value > other.value ? 1 : 0;
}

} // namespace inet
