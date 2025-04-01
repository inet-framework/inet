//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.contract;

//
// Interface for the upper interface of
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
// Interface for the lower interface of
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
// Interface for both upper and lower
// interfaces of different link layers.
//
moduleinterface ILinkLayer extends ILinkLayerUpper, ILinkLayerLower
{
    parameters:
        @display("i=block/rxtx");
}

