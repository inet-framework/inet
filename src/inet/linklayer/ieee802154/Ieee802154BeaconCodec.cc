// SPDX-License-Identifier: LGPL-3.0-or-later
//
#include "inet/linklayer/ieee802154/Ieee802154BeaconCodec.h"

namespace inet {

std::vector<uint8_t> Ieee802154BeaconCodec::encode(const Content& content)
{
    if (content.gtsDescriptors.size() % 3 || content.gtsDescriptors.size() > 21 || content.pendingAddresses.size() > 7)
        throw cRuntimeError("Invalid IEEE 802.15.4 beacon GTS/pending count");
    unsigned int shortCount = 0, extendedCount = 0;
    for (const auto& address : content.pendingAddresses) {
        if (address.isShort() && !address.isShortBroadcast() && !extendedCount)
            ++shortCount;
        else if (address.isExtended())
            ++extendedCount;
        else
            throw cRuntimeError("Invalid IEEE 802.15.4 pending address order or mode");
    }
    // IEEE Std 802.15.4-2024, 7.3.1.4–7 and Figures 7-7 through 7-13.
    std::vector<uint8_t> bytes{uint8_t(content.superframeSpecification), uint8_t(content.superframeSpecification >> 8),
            uint8_t(content.gtsDescriptors.size() / 3 | (content.gtsPermit ? 0x80 : 0))};
    if (!content.gtsDescriptors.empty()) {
        bytes.push_back(content.gtsDirections & 0x7f);
        bytes.insert(bytes.end(), content.gtsDescriptors.begin(), content.gtsDescriptors.end());
    }
    bytes.push_back(shortCount | (extendedCount << 4));
    for (const auto& address : content.pendingAddresses) {
        auto value = address.isShort() ? uint64_t(address.getShort()) : address.getExtended();
        for (unsigned int i = 0; i < (address.isShort() ? 2u : 8u); ++i)
            bytes.push_back(value >> (8 * i));
    }
    bytes.insert(bytes.end(), content.payload.begin(), content.payload.end());
    return bytes;
}

Ieee802154BeaconCodec::DecodeResult Ieee802154BeaconCodec::decode(const std::vector<uint8_t>& bytes)
{
    if (bytes.size() < 4)
        return Error::TRUNCATED;
    Content content;
    content.superframeSpecification = bytes[0] | uint16_t(bytes[1]) << 8;
    content.gtsPermit = bytes[2] & 0x80;
    size_t descriptors = bytes[2] & 7;
    size_t position = 3;
    if (descriptors) {
        if (bytes.size() < position + 1 + 3 * descriptors + 1)
            return Error::TRUNCATED;
        content.gtsDirections = bytes[position++] & 0x7f;
        content.gtsDescriptors.assign(bytes.begin() + position, bytes.begin() + position + 3 * descriptors);
        position += 3 * descriptors;
    }
    auto specification = bytes[position++];
    unsigned int shortCount = specification & 7, extendedCount = (specification >> 4) & 7;
    if (shortCount + extendedCount > 7)
        return Error::MALFORMED;
    if (bytes.size() < position + 2 * shortCount + 8 * extendedCount)
        return Error::TRUNCATED;
    auto readLe = [&](unsigned int length) {
        uint64_t value = 0;
        for (unsigned int i = 0; i < length; ++i)
            value |= uint64_t(bytes[position++]) << (8 * i);
        return value;
    };
    for (unsigned int i = 0; i < shortCount; ++i) {
        auto address = Ieee802154Address::fromShort(readLe(2));
        if (address.isShortBroadcast())
            return Error::MALFORMED;
        content.pendingAddresses.push_back(address);
    }
    for (unsigned int i = 0; i < extendedCount; ++i)
        content.pendingAddresses.push_back(Ieee802154Address::fromExtended(readLe(8)));
    content.payload.assign(bytes.begin() + position, bytes.end());
    return content;
}

} // namespace inet
