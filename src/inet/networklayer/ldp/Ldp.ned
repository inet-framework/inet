//
// Copyright (C) 2005 Vojta Janota
// Copyright (C) 2004 Andras Varga
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.networklayer.ldp;

import inet.common.SimpleModule;
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
// Model simplification: RFC 5036 allows a PDU to carry multiple LDP messages;
// this model always sends exactly one message per PDU, so the chunk length
// computed for each outgoing packet is simply the 10-byte PDU header plus the
// length of that single message (its 8-byte message header plus its TLVs).
//
// @author Vojta Janota
//
simple Ldp extends SimpleModule like IApp
{
    parameters:
        @class(Ldp);
        string interfaceTableModule;   // The path to the InterfaceTable module
        string routingTableModule;
        string libTableModule;
        string tedModule;
        double holdTime @unit(s) = default(15s);
        double helloInterval @unit(s) = default(5s);
        bool advertiseImplicitNull = default(true); // penultimate hop popping: advertise the implicit null label (RFC 3032) instead of allocating a local pop entry when acting as egress for a FEC
        @display("i=block/control");
        @signal[tedChanged](type=inet::TedChangeInfo);
        @signal[sessionUp](type=long); // peer's router id (as a 32-bit integer) when an LDP session comes up
        @signal[sessionDown](type=long); // peer's router id (as a 32-bit integer) when an LDP session goes down
        @signal[fecBindingCount](type=long); // fecUp.size() + fecDown.size(), emitted whenever a label binding is added or removed
        @statistic[sessionUp](title="LDP sessions established"; record=count,"vector(constant1)"; interpolationmode=none);
        @statistic[sessionDown](title="LDP sessions torn down"; record=count,"vector(constant1)"; interpolationmode=none);
        @statistic[fecBindingCount](title="number of FEC label bindings"; record=last,vector; interpolationmode=none);
        @lifecycleSupport;
        double stopOperationExtraTime @unit(s) = default(-1s);    // Extra time after lifecycle stop operation finished
        double stopOperationTimeout @unit(s) = default(2s);    // Timeout value for lifecycle stop operation

    gates:
        input socketIn @labels(TcpCommand/up,UdpCommand/up);
        output socketOut @labels(TcpCommand/down,UdpCommand/down);
}

