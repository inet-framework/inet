//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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
    NO_CARRIER                = 5;
    NO_INTERFACE_FOUND        = 6;
    NO_ROUTE_FOUND            = 7;
    NOT_ADDRESSED_TO_US       = 8;
    QUEUE_OVERFLOW            = 9;
    RETRY_LIMIT_REACHED       = 10;
    LIFETIME_EXPIRED          = 11;
    CONGESTION                = 12;
    NO_PROTOCOL_FOUND         = 13;
    NO_PORT_FOUND             = 14;
    DUPLICATE_DETECTED        = 15;
    OTHER_PACKET_DROP         = -1;
}

class PacketDropDetails extends cObject
{
    PacketDropReason reason;
    int limit = -1;
}
