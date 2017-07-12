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
    INTERFACE_DOWN              = 3;
    FORWARDING_DISABLED         = 4;
    NO_INTERFACE_FOUND          = 5;
    NO_ROUTE_FOUND              = 6;
    ADDRESS_RESOLUTION_FAILED   = 7;
    PACKET_INCORRECTLY_RECEIVED = 8;
    PACKET_NOT_FOR_US           = 9;
}

class PacketDropDetails
{
    int reason @enum(PacketDropReason) = -1;
}