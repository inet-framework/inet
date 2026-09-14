//
// Copyright (C) 2026
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_IEEE80211OFDMSIGNALFIELD_H
#define __INET_IEEE80211OFDMSIGNALFIELD_H

#include <cstdint>

namespace inet {

namespace physicallayer {

struct Ieee80211OfdmSignalField
{
    uint8_t rate = 0;
    bool reserved = false;
    uint16_t length = 0;
    bool parity = false;
    uint8_t tail = 0;
};

inline uint32_t packIeee80211OfdmSignalField(uint8_t rate, bool reserved, uint16_t length, bool parity, uint8_t tail)
{
    // IEEE Std 802.11-2024 Figure 17-5 and 17.3.4: SIGNAL bit 0 is RATE bit
    // R1 and the LSB is transmitted first.
    return static_cast<uint32_t>(rate & 0xF) |
            (reserved ? 0x10 : 0) |
            (static_cast<uint32_t>(length & 0xFFF) << 5) |
            (parity ? 0x20000 : 0) |
            (static_cast<uint32_t>(tail & 0x3F) << 18);
}

inline Ieee80211OfdmSignalField unpackIeee80211OfdmSignalField(uint32_t signal)
{
    Ieee80211OfdmSignalField field;
    field.rate = static_cast<uint8_t>(signal & 0xF);
    field.reserved = (signal & 0x10) != 0;
    field.length = static_cast<uint16_t>((signal >> 5) & 0xFFF);
    field.parity = (signal & 0x20000) != 0;
    field.tail = static_cast<uint8_t>((signal >> 18) & 0x3F);
    return field;
}

inline bool computeIeee80211OfdmSignalParity(uint8_t rate, bool reserved, uint16_t length)
{
    // IEEE Std 802.11-2024, 17.3.4.4: bit 17 gives even parity over bits 0-16.
    uint32_t protectedBits = packIeee80211OfdmSignalField(rate, reserved, length, false, 0);
    bool parity = false;
    while (protectedBits != 0) {
        parity = !parity;
        protectedBits &= protectedBits - 1;
    }
    return parity;
}

inline bool isIeee80211OfdmSignalValid(const Ieee80211OfdmSignalField& field)
{
    // IEEE Std 802.11-2024, 17.3.4.2 and 17.3.12: all eight defined
    // RATE codes are odd four-bit values. Reserved is ignored on receive,
    // but still participates in the parity check (17.3.4.4).
    return (field.rate & 1) != 0 &&
            field.parity == computeIeee80211OfdmSignalParity(field.rate, field.reserved, field.length);
}

inline Ieee80211OfdmSignalField unpackIeee80211OfdmSignalField(uint8_t byte0, uint8_t byte1, uint8_t byte2)
{
    uint32_t signal = static_cast<uint32_t>(byte0) |
            (static_cast<uint32_t>(byte1) << 8) |
            (static_cast<uint32_t>(byte2) << 16);
    return unpackIeee80211OfdmSignalField(signal);
}

} // namespace physicallayer

} // namespace inet

#endif
