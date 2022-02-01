//
// Copyright (C) 2005 Vojtech Janota, Andras Varga
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.networklayer.ted;

//
// This module implements a very minimalistic link state routing protocol.
// Apart from the basic topology information, the current link usage
// is distributed to all participants in the network (by means of flooding).
//
// Collaborations
//
// The link state database is kept in the ~Ted module.
//
simple LinkStateRouting
{
    parameters:
        string interfaceTableModule;   // The path to the InterfaceTable module
        string routingTableModule;
        string tedModule;
        string peers;
        @display("i=block/network2");
    gates:
        input ipIn @labels(Ipv4ControlInfo/up);
        output ipOut @labels(Ipv4ControlInfo/down);
}

