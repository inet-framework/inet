//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_IEEE80211PLCPCRC_H
#define __INET_IEEE80211PLCPCRC_H

#include "inet/common/INETDefs.h"

namespace inet {
namespace physicallayer {

// IEEE Std 802.11-2024, 15.3.3.7; IEEE Std 802.11-1999, 14.3.2.2.3.
// Process each protected octet in transmission order (least significant bit
// first). The returned reflected, complemented remainder is serialized LE.
inline uint16_t computeIeee80211PlcpCrc(const uint8_t *bytes, size_t length)
{
    uint16_t crc = 0xffff;
    for (size_t i = 0; i < length; i++) {
        crc ^= bytes[i];
        for (int bit = 0; bit < 8; bit++)
            crc = (crc >> 1) ^ ((crc & 1) ? 0x8408 : 0);
    }
    return crc ^ 0xffff;
}

} // namespace physicallayer
} // namespace inet

#endif
