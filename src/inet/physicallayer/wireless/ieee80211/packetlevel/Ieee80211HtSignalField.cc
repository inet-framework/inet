// SPDX-License-Identifier: LGPL-3.0-or-later

#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211HtSignalField.h"

#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211HtMode.h"

namespace inet {
namespace physicallayer {

uint64_t packIeee80211HtSignalField(const Ieee80211HtPhyHeader& header)
{
    // IEEE Std 802.11-2024, Figure 19-6: each octet holds successive wire bits, LSB first.
    auto length = header.getLengthField().get<B>();
    if (length < 0 || length > 65535 || header.getMcs() > 127 || header.getStbc() > 3 ||
            header.getExtensionSpatialStreams() > 3 || header.getTail() > 63)
        throw cRuntimeError("HT-SIG field is outside its encoded range");
    return uint64_t(header.getMcs()) | (uint64_t(header.getChannelWidth40()) << 7) |
            (uint64_t(length) << 8) | (uint64_t(header.getSmoothing()) << 24) |
            (uint64_t(header.getNotSounding()) << 25) | (uint64_t(header.getReserved()) << 26) |
            (uint64_t(header.getAggregation()) << 27) | (uint64_t(header.getStbc()) << 28) |
            (uint64_t(header.getFecCoding()) << 30) | (uint64_t(header.getShortGi()) << 31) |
            (uint64_t(header.getExtensionSpatialStreams()) << 32) | (uint64_t(header.getCrc()) << 34) |
            (uint64_t(header.getTail()) << 42);
}

uint8_t computeIeee80211HtSignalCrc(const Ieee80211HtPhyHeader& header)
{
    // IEEE Std 802.11-2024, 19.3.9.4.4: process bits 0..33, emit complemented C7 first.
    auto bits = packIeee80211HtSignalField(header);
    uint8_t remainder = 0xFF;
    for (int bit = 0; bit < 34; bit++) {
        bool feedback = ((remainder >> 7) ^ (bits >> bit)) & 1;
        remainder <<= 1;
        if (feedback)
            remainder ^= 0x07;
    }
    uint8_t crc = 0;
    uint8_t complemented = ~remainder;
    for (int bit = 0; bit < 8; bit++)
        crc |= ((complemented >> (7 - bit)) & 1) << bit;
    return crc;
}

bool isIeee80211HtSignalValid(const Ieee80211HtPhyHeader& header, const IIeee80211Mode *mode)
{
    auto htMode = dynamic_cast<const Ieee80211HtMode *>(mode);
    if (htMode == nullptr || header.getChunkLength() != b(48) || !header.isCorrect() ||
            !header.isComplete() || !header.isProperlyRepresented() ||
            header.getLengthField() <= B(0) || header.getLengthField() > B(65535) || header.getMcs() > 31 ||
            header.getStbc() != 0 || header.getFecCoding() || header.getExtensionSpatialStreams() != 0 ||
            !header.getNotSounding() || !header.getReserved() || header.getTail() != 0)
        return false;
    // Packet-level profile: BCC, no STBC, no extension streams, no sounding.
    auto dataMode = check_and_cast<const Ieee80211HtDataMode *>(mode->getDataMode());
    return header.getMcs() == mode->getHtMcsIndex() &&
            header.getChannelWidth40() == (dataMode->getBandwidth() == MHz(40)) &&
            header.getShortGi() == (dataMode->getGuardIntervalType() == Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) &&
            header.getCrc() == computeIeee80211HtSignalCrc(header);
}

} // namespace physicallayer
} // namespace inet
