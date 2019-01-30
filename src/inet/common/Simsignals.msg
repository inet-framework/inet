//
// This program is property of its copyright holder. All rights reserved.
//

import inet.common.INETDefs;

namespace inet;

enum PacketDropReason
{
    ADDRESS_RESOLUTION_FAILED = 0;
    FORWARDING_DISABLED       = 1;
    HOP_LIMIT_REACHED         = 2;
    INCORRECTLY_RECEIVED      = 3;
    INTERFACE_DOWN            = 4;
    NO_INTERFACE_FOUND        = 5;
    NO_ROUTE_FOUND            = 6;
    NOT_ADDRESSED_TO_US       = 7;
    QUEUE_OVERFLOW            = 8;
    RETRY_LIMIT_REACHED       = 9;
    LIFETIME_EXPIRED          = 10;
    CONGESTION                = 11;
    NO_PROTOCOL_FOUND         = 12;
    NO_PORT_FOUND             = 13;
    DUPLICATE_DETECTED        = 14;
    OTHER_PACKET_DROP         = 99;
}

class PacketDropDetails extends cObject
{
    PacketDropReason reason;
    int limit = -1;
}