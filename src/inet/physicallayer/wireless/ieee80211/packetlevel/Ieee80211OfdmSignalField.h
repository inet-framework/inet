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
