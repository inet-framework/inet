//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.routing.contract;

//
// Interface for the IS-IS routing protocol module. Unlike IP-based routing
// protocols (~IEigrp, ~IOspf), IS-IS exchanges its PDUs directly over the link
// layer (IEEE 802.2 LLC), so it connects to the link-layer message dispatcher
// rather than to the network layer.
//
moduleinterface IIsis
{
    parameters:
        @display("i=block/network");
    gates:
        input socketIn;
        output socketOut;
}
