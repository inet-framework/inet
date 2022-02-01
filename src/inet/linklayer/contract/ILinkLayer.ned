//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.contract;

//
// This module interface provides an abstraction for the upper interface of
// different link layers.
//
moduleinterface ILinkLayerUpper
{
    parameters:
        @display("i=block/rxtx");
    gates:
        input upperLayerIn @labels(INetworkHeader/down);
        output upperLayerOut @labels(INetworkHeader/up);
}

//
// This module interface provides an abstraction for the lower interface of
// different link layers.
//
moduleinterface ILinkLayerLower
{
    parameters:
        @display("i=block/rxtx");
    gates:
        input lowerLayerIn @labels(ILinkFrame/up);
        output lowerLayerOut @labels(ILinkFrame/down);
}

//
// This module interface provides an abstraction for both upper and lower
// interfaces of different link layers.
//
moduleinterface ILinkLayer extends ILinkLayerUpper, ILinkLayerLower
{
    parameters:
        @display("i=block/rxtx");
}

