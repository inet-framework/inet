//
// Copyright (C) 2020 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
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
