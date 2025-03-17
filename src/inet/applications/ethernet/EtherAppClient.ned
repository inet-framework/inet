//
// Copyright (C) 2003 Andras Varga; CTIE, Monash University, Australia
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.applications.ethernet;

//
// Client application that generates request-response traffic directly at the data
// link layer, bypassing the network and transport layers. Designed for testing and
// benchmarking Ethernet networks, 802.11 wireless networks, and other Layer 2
// technologies that support IEEE 802 Service Access Points (SAPs).
//
// Periodically sends request packets containing EtherAppReq chunks to a specified
// destination MAC address, with configurable timing and size parameters. Each request
// specifies the desired size of the corresponding response. Collects comprehensive
// statistics including packet counts, bytes transferred, and end-to-end delay
// measurements. Supports lifecycle operations for controlled startup and shutdown
// in simulation scenarios.
//
// @see ~EtherAppServer, ~Ieee8022Llc, ~EtherAppReq, ~EtherAppResp
//
simple EtherAppClient
{
    parameters:
        string interfaceTableModule;
        string destAddress = default("");  // Destination MAC address, or module path name of destination station; empty means off
        string interface = default("");  // Outgoing interface name; empty accepted only when the destAddress is empty
        double startTime @unit(s) = default(this.sendInterval);  // Time of sending the first request
        double stopTime @unit(s) = default(-1s);  // Time of finishing sending, negative values mean forever
        int localSAP = default(0xf0);  // Local service access point
        int remoteSAP = default(0xf1);  // Remote service access point
        volatile double sendInterval @unit(s) = default(uniform(0s,1s));  // Interval between sending requests
        volatile int reqLength @unit(B) = default(100B);  // Length of request packets
        volatile int respLength @unit(B) = default(1KiB);  // Length of response packets
        @lifecycleSupport;
        double stopOperationExtraTime @unit(s) = default(-1s);    // Extra time after lifecycle stop operation finished
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
