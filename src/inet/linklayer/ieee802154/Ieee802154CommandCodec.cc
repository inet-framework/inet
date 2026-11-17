// SPDX-License-Identifier: LGPL-3.0-or-later
//
#include "inet/linklayer/ieee802154/Ieee802154CommandCodec.h"

#include <type_traits>

namespace inet {

std::vector<uint8_t> Ieee802154CommandCodec::encode(const Command& command)
{
    // IEEE Std 802.15.4-2024, Table 7-11 and the command-specific Content formats.
    return std::visit([](const auto& value) -> std::vector<uint8_t> {
        using T = std::decay_t<decltype(value)>;
        if constexpr (std::is_same_v<T, AssociationRequest>)
            return {1, value.capabilityInformation};
        else if constexpr (std::is_same_v<T, AssociationResponse>)
            return {2, uint8_t(value.shortAddress), uint8_t(value.shortAddress >> 8), value.status};
        else if constexpr (std::is_same_v<T, DisassociationNotification>)
            return {3, value.reason};
        else if constexpr (std::is_same_v<T, DataRequest>)
            return {4};
        else if constexpr (std::is_same_v<T, PanConflictNotification>)
            return {5};
        else if constexpr (std::is_same_v<T, OrphanNotification>)
            return {6};
        else if constexpr (std::is_same_v<T, BeaconRequest>)
            return {7};
        else
            return {8, uint8_t(value.panId), uint8_t(value.panId >> 8),
                    uint8_t(value.coordinatorShortAddress), uint8_t(value.coordinatorShortAddress >> 8),
                    value.channelNumber, uint8_t(value.shortAddress), uint8_t(value.shortAddress >> 8)};
    }, command);
}

Ieee802154CommandCodec::DecodeResult Ieee802154CommandCodec::decode(const std::vector<uint8_t>& bytes)
{
    if (bytes.empty())
        return Error::TRUNCATED;
    // IEEE Std 802.15.4-2024, Table 7-11: other IDs belong to unselected procedures.
    if (bytes[0] < 1 || bytes[0] > 8)
        return Error::UNSUPPORTED;
    const size_t lengths[] = {0, 2, 4, 2, 1, 1, 1, 1, 8};
    auto expected = lengths[bytes[0]];
    if (bytes.size() < expected)
        return Error::TRUNCATED;
    if (bytes[0] == 8 && bytes.size() == 9) {
        // Figure 7-30: bit 7 selects extended channel numbering, outside this profile.
        if (bytes[8] & 0x80)
            return Error::UNSUPPORTED;
    }
    else if (bytes.size() != expected)
        return Error::MALFORMED;
    auto readLe = [&](size_t offset) { return uint16_t(bytes[offset] | uint16_t(bytes[offset + 1]) << 8); };
    switch (bytes[0]) {
        case 1: return Command(AssociationRequest{bytes[1]});
        case 2: return Command(AssociationResponse{readLe(1), bytes[3]});
        case 3: return Command(DisassociationNotification{bytes[1]});
        case 4: return Command(DataRequest{});
        case 5: return Command(PanConflictNotification{});
        case 6: return Command(OrphanNotification{});
        case 7: return Command(BeaconRequest{});
        case 8: return Command(CoordinatorRealignment{readLe(1), readLe(3), bytes[5], readLe(6)});
    }
    throw cRuntimeError("Invalid IEEE 802.15.4 command dispatch");
}

} // namespace inet
