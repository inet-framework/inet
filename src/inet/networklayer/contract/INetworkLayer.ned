//
// Copyright (C) 2012 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.networklayer.contract;

//
// Interface for different network layers.
//
moduleinterface INetworkLayerUpper
{
    parameters:
        @display("i=block/fork");
    gates:
        input transportIn @labels(ITransportPacket/down);
        output transportOut @labels(ITransportPacket/up);
}

//
// TODO
//
moduleinterface INetworkLayerLower
{
    parameters:
        @display("i=block/fork");
    gates:
        input ifIn @labels(INetworkHeader/up);
        output ifOut @labels(INetworkHeader/down);
}

//
// TODO
//
moduleinterface INetworkLayer extends INetworkLayerUpper, INetworkLayerLower
{
    parameters:
        @display("i=block/fork");
}

