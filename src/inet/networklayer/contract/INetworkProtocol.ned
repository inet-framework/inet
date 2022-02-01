//
// Copyright (C) 2012 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.networklayer.contract;

//
// This interface provides an abstraction for different network protocols.
//
moduleinterface INetworkProtocol
{
    parameters:
        @display("i=block/fork");
    gates:
        input transportIn @labels(INetworkProtocolControlInfo/down);
        output transportOut @labels(INetworkProtocolControlInfo/up);
        input queueIn @labels(INetworkHeader);
        output queueOut @labels(INetworkHeader);
}

