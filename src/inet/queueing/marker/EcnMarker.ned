//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.marker;

import inet.queueing.base.PacketMarkerBase;

//
// Sets Explicit Congestion Notification (ECN) bits in IPv4 header based on the
// attached EcnReq tag on the packets.
//
simple EcnMarker extends PacketMarkerBase
{
    parameters:
        @class(EcnMarker);
}

