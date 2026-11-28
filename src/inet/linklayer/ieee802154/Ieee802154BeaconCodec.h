// SPDX-License-Identifier: LGPL-3.0-or-later
//
#ifndef __INET_IEEE802154BEACONCODEC_H
#define __INET_IEEE802154BEACONCODEC_H

#include <variant>
#include <vector>
#include "inet/linklayer/ieee802154/Ieee802154Address.h"

namespace inet {

/** Legacy beacon content, including bounded discovery of periodic-beacon sources. */
class INET_API Ieee802154BeaconCodec
{
  public:
    struct Content {
        uint16_t superframeSpecification = 0x0fff;
        bool gtsPermit = false;
        uint8_t gtsDirections = 0; ///< Low seven bits; present only with descriptors.
        std::vector<uint8_t> gtsDescriptors; ///< Complete three-octet descriptors, at most seven.
        std::vector<Ieee802154Address> pendingAddresses; ///< Short addresses precede extended addresses on wire.
        std::vector<uint8_t> payload;
    };
    enum class Error { TRUNCATED, MALFORMED };
    using DecodeResult = std::variant<Content, Error>;
    static std::vector<uint8_t> encode(const Content& content);
    static DecodeResult decode(const std::vector<uint8_t>& bytes);
};

} // namespace inet
#endif
