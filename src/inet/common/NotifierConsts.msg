//
// This program is property of its copyright holder. All rights reserved.
//

cplusplus{{
#include "inet/common/INETDefs.h"
}}

namespace inet;

enum PacketDropReason
{
    QUEUE_OVERFLOW              = 0;
    RETRY_LIMIT_REACHED         = 1;
    HOP_LIMIT_REACHED           = 2;
    FORWARDING_DISABLED         = 3;
    NO_INTERFACE_FOUND          = 4;
    NO_ROUTE_FOUND              = 5;
    ADDRESS_RESOLUTION_FAILED   = 6;
    PACKET_INCORRECTLY_RECEIVED = 7;
}

class PacketDropDetails
{
    int reason @enum(PacketDropReason) = -1;
}