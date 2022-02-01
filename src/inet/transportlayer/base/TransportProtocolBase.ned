//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.transportlayer.base;

import inet.common.LayeredProtocolBase;

//
// Module base for different transport protocols.
//
simple TransportProtocolBase extends LayeredProtocolBase
{
    gates:
        input upperLayerIn[];
        output upperLayerOut[];
        input lowerLayerIn @labels(ITransportPacket/up);
        output lowerLayerOut @labels(ITransportPacket/down);
}

