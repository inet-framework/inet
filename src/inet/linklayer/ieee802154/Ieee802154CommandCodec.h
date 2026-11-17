// SPDX-License-Identifier: LGPL-3.0-or-later
//
#ifndef __INET_IEEE802154COMMANDCODEC_H
#define __INET_IEEE802154COMMANDCODEC_H

#include <variant>
#include <vector>
#include "inet/common/INETDefs.h"

namespace inet {

/** Command ID plus Content; MHR validation and procedure state belong to the MAC. */
class INET_API Ieee802154CommandCodec
{
  public:
    struct AssociationRequest { uint8_t capabilityInformation; };
    struct AssociationResponse { uint16_t shortAddress; uint8_t status; };
    struct DisassociationNotification { uint8_t reason; };
    struct DataRequest {};
    struct PanConflictNotification {};
    struct OrphanNotification {};
    struct BeaconRequest {};
    struct CoordinatorRealignment {
        uint16_t panId;
        uint16_t coordinatorShortAddress;
        uint8_t channelNumber; ///< Selected page-0 channel representation.
        uint16_t shortAddress;
    };
    using Command = std::variant<AssociationRequest, AssociationResponse, DisassociationNotification,
            DataRequest, PanConflictNotification, OrphanNotification, BeaconRequest, CoordinatorRealignment>;
    enum class Error { TRUNCATED, MALFORMED, UNSUPPORTED };
    using DecodeResult = std::variant<Command, Error>;
    static std::vector<uint8_t> encode(const Command& command);
    static DecodeResult decode(const std::vector<uint8_t>& bytes);
};

} // namespace inet
#endif
