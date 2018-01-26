//
// (C) 2005 Vojta Janota
// (C) 2004 Andras Varga
//
// This library is free software, you can redistribute it
// and/or modify
// it under  the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation;
// either version 2 of the License, or any later version.
// The library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//


package inet.networklayer.ldp;

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
// All Ldp message types are subclassed from ~LdpPacket, and include
// ~LdpHello, ~LdpNotify, ~LdpLabelRequest, ~LdpIni and ~LdpAddress.
//
// Message processing in the ~Ldp module is explained in
// <a href="ldp-processing.html">~LDP Message Processing</a>
//
// @author Andras Varga, Vojta Janota
//
simple Ldp      //FIXME like IApp
{
    parameters:
        string interfaceTableModule;   // The path to the InterfaceTable module
        string routingTableModule;
        string libTableModule;
        string tedModule;
        double holdTime @unit(s) = default(15s);
        double helloInterval @unit(s) = default(5s);
        string dataTransferMode @enum("bytecount", "object", "bytestream") = default("bytecount");
        @display("i=block/control");
    gates:
        //FIXME use socketIn/socketOut gates instead of tcpIn/Out, udpIn/Out
        input tcpIn @labels(TcpCommand/up);
        input udpIn @labels(UdpControlInfo/up);
        output tcpOut @labels(TcpCommand/down);
        output udpOut @labels(UdpControlInfo/down);
}
