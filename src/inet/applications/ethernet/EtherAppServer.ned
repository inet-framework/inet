//
// Copyright (C) 2003 Andras Varga; CTIE, Monash University, Australia
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.applications.ethernet;

//
// Server component that responds to requests from EtherAppClient applications
// operating directly at the data link layer. Processes incoming request packets
// containing EtherAppReq chunks and generates response packets with EtherAppResp
// chunks sized according to the client's specifications.
//
// Designed to work in tandem with EtherAppClient for testing and benchmarking
// Ethernet networks and other Layer 2 technologies. Maintains statistics on
// traffic patterns including packet counts, bytes transferred, and end-to-end
// delay measurements. The server listens on a configurable Service Access Point
// (SAP) identifier and should be connected directly to an Ieee8022Llc module
// to handle the LLC encapsulation and addressing.
//
// @see ~EtherAppClient, ~Ieee8022Llc, ~EtherAppReq, ~EtherAppResp
//
simple EtherAppServer
{
    parameters:
        int localSAP = default(0xf1);
        @lifecycleSupport;
        double stopOperationExtraTime @unit(s) = default(-1s);    // Extra time after lifecycle stop operation is finished
        double stopOperationTimeout @unit(s) = default(2s);    // Timeout value for lifecycle stop operation
        @display("i=block/app");
        @signal[packetSent](type=inet::Packet);
        @signal[packetReceived](type=inet::Packet);
        @statistic[packetReceived](title="packets received"; source=packetReceived; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[packetSent](title="packets sent"; source=packetSent; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[endToEndDelay](title="end-to-end delay"; source="dataAge(packetReceived)"; unit=s; record=histogram,vector; interpolationmode=none);
    gates:
        input in @labels(Ieee802Ctrl/up) @messageKinds(inet::SocketStatusInd);
        output out @labels(Ieee802Ctrl/down) @messageKinds(inet::SocketCommandCode);
}
