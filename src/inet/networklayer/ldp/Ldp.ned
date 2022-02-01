//
// Copyright (C) 2005 Vojta Janota
// Copyright (C) 2004 Andras Varga
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.networklayer.ldp;

import inet.applications.contract.IApp;

//
// Handles and processes LDP messages.
//
// The LDP protocol is used by one LSR (Label Switched Router) to inform
// another LSR of the label bindings it has made. The LSR uses this protocol
// to establish label switched paths through a network by mapping network layer
// routing information directly to data-link layer switched paths.
//
// Operations
//
// All LDP message types are subclassed from ~LdpPacket, and include
// ~LdpHello, ~LdpNotify, ~LdpLabelRequest, ~LdpIni and ~LdpAddress.
//
// Message processing in the ~Ldp module is explained in
// <a href="ldp-processing.html">LDP Message Processing</a>
//
// @author Vojta Janota
//
simple Ldp like IApp
{
    parameters:
        string interfaceTableModule;   // The path to the InterfaceTable module
        string routingTableModule;
        string libTableModule;
        string tedModule;
        double holdTime @unit(s) = default(15s);
        double helloInterval @unit(s) = default(5s);
        @display("i=block/control");
        @lifecycleSupport;
        double stopOperationExtraTime @unit(s) = default(-1s);    // extra time after lifecycle stop operation finished
        double stopOperationTimeout @unit(s) = default(2s);    // timeout value for lifecycle stop operation

    gates:
        input socketIn @labels(TcpCommand/up,UdpControlInfo/up);
        output socketOut @labels(TcpCommand/down,UdpControlInfo/down);
}

