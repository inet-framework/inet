//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.applications.ipapp;

//
// Provides a bridge between application-level traffic and IP protocol, handling
// the details of socket management and packet transmission. Acts as an adapter
// that converts between application data and IP socket commands, simplifying the
// development of IP-based applications by abstracting away the complexities of
// socket operations.
//
// Offers configuration options for IP communication including local and destination
// addressing and protocol selection. The module maintains statistics on sent and
// received packets, enabling detailed analysis of traffic patterns. Designed to be
// used as a component within modular IP applications, particularly those built with
// the queueing model architecture.
//
// @see ~IpApp, ~IpSocket
//
simple IpSocketIo
{
    parameters:
        string interfaceTableModule;
        string localAddress = default("");
        string destAddress = default("");
        string protocol = default("");
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
