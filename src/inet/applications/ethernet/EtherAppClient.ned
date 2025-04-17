//
// Copyright (C) 2003 Andras Varga; CTIE, Monash University, Australia
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.applications.ethernet;

import inet.common.SimpleModule;

//
// A simple traffic generator for the Ethernet model and the 802.11 model,
// and generally for any L2 model that accepts ~Ieee802SapReq tags on the
// packets. It generates packets containing ~EtherAppReq chunks. It should
// be connected directly to ~Ieee8022Llc.
//
simple EtherAppClient extends SimpleModule
{
    parameters:
        @class(EtherAppClient);
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

