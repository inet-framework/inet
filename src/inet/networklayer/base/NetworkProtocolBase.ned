//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.networklayer.base;

import inet.common.LayeredProtocolBase;

//
// Module base for different network protocols.
//
simple NetworkProtocolBase extends LayeredProtocolBase
{
    parameters:
        string interfaceTableModule;
        @display("i=block/fork");
    gates:
        input transportIn @labels(INetworkProtocolControlInfo/down);
        output transportOut @labels(INetworkProtocolControlInfo/up);
        input queueIn @labels(INetworkHeader);
        output queueOut @labels(INetworkHeader);
}

