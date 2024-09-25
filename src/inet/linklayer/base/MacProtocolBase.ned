//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.base;

import inet.common.LayeredProtocolBase;

//
// Module base for different MAC protocols.
//
simple MacProtocolBase extends LayeredProtocolBase
{
    parameters:
        string interfaceTableModule;
        @display("i=block/rxtx");
    gates:
        input upperLayerIn @interface[inet::queueing::IActivePacketSink](arguments=null) @labels(INetworkHeader/down);
        output upperLayerOut @labels(INetworkHeader/up);
        input lowerLayerIn @interface[inet::queueing::IPassivePacketSink] @labels(ILinkLayerFrame/up);
        output lowerLayerOut @labels(ILinkLayerFrame/down);
}

