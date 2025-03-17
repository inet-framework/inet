//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.applications.ieee8022;

//
// Provides a bridge between application-level traffic and IEEE 802.2 Logical Link
// Control (LLC) protocol, handling the details of socket management and packet
// transmission. Acts as an adapter that converts between application data and LLC
// socket commands, simplifying the development of data link layer applications by
// abstracting away the complexities of LLC socket operations.
//
// Offers configuration options for LLC communication including interface selection,
// local and remote MAC addresses, and Service Access Point (SAP) identifiers. The
// module maintains statistics on sent and received packets, enabling detailed
// analysis of traffic patterns. Designed to be used as a component within modular
// LLC applications, particularly those built with the queueing model architecture.
//
// @see ~Ieee8022LlcApp, ~Ieee8022LlcSourceApp, ~Ieee8022LlcSinkApp, ~Ieee8022LlcSocket
//
simple Ieee8022LlcSocketIo
{
    parameters:
        string interfaceTableModule;
        string interface = default("");
        string localAddress = default("");
        string remoteAddress = default("");
        int localSap = default(-1);
        int remoteSap = default(-1);
        double stopOperationExtraTime @unit(s) = default(-1s);    // Extra time after lifecycle stop operation finishes
        double stopOperationTimeout @unit(s) = default(2s);    // Timeout value for lifecycle stop operation
        @lifecycleSupport();
        @display("i=block/socket");
        @signal[packetSent](type=inet::Packet);
        @signal[packetReceived](type=inet::Packet);
        @statistic[packetSent](title="packets sent"; source=packetSent; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[packetReceived](title="packets received"; source=packetReceived; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
    gates:
        input trafficIn;
        output trafficOut;
        input socketIn;
        output socketOut;
}
