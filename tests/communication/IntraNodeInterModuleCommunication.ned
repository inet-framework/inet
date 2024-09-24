//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.communication;

simple PacketSink
{
    gates:
        input in;
}

simple TcpApp
{
    gates:
        input socketIn;
        output socketOut;
}

simple Tcp
{
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input lowerLayerIn;
        output lowerLayerOut;
}

// TCP application directly connects to TCP protocol
network TcpNetwork1
{
    submodules:
        app: TcpApp;
        tcp: Tcp;
        sink: PacketSink;
    connections allowunconnected:
        app.socketOut --> tcp.upperLayerIn;
        app.socketIn <-- tcp.upperLayerOut;
        tcp.lowerLayerOut --> sink.in;
}

simple TcpProcessingDelay
{
    gates:
        input in;
        output out;
}

// delay TCP socket operations and TCP SDUs
network TcpNetwork2
{
    submodules:
        app: TcpApp;
        delay: TcpProcessingDelay;
        tcp: Tcp;
        sink: PacketSink;
    connections allowunconnected:
        app.socketOut --> delay.in;
        delay.out --> tcp.upperLayerIn;
        app.socketIn <-- tcp.upperLayerOut;
        tcp.lowerLayerOut --> sink.in;
}

simple Dispatcher
{
    gates:
        input in[];
        output out[];
}

// add dispatcher between delay and TCP
network TcpNetwork3
{
    submodules:
        app: TcpApp;
        delay: TcpProcessingDelay;
        dispatcher: Dispatcher;
        tcp: Tcp;
        sink: PacketSink;
    connections allowunconnected:
        app.socketOut --> delay.in;
        delay.out --> dispatcher.in++;
        app.socketIn <-- dispatcher.out++;
        dispatcher.out++ --> tcp.upperLayerIn;
        dispatcher.in++ <-- tcp.upperLayerOut;
        tcp.lowerLayerOut --> sink.in;
}

// swap delay and dispatcher
network TcpNetwork4
{
    submodules:
        app: TcpApp;
        dispatcher: Dispatcher;
        delay: TcpProcessingDelay;
        tcp: Tcp;
        sink: PacketSink;
    connections allowunconnected:
        app.socketOut --> dispatcher.in++;
        app.socketIn <-- dispatcher.out++;
        dispatcher.out++ --> delay.in;
        delay.out --> tcp.upperLayerIn;
        dispatcher.in++ <-- tcp.upperLayerOut;
        tcp.lowerLayerOut --> sink.in;
}

simple PcapRecorder
{
    gates:
        input in;
        output out;
}

// move delay after dispatcher and add PCAP recorder before dispatcher
network TcpNetwork5
{
    submodules:
        app: TcpApp;
        recorder: PcapRecorder;
        dispatcher: Dispatcher;
        delay: TcpProcessingDelay;
        tcp: Tcp;
        sink: PacketSink;
    connections allowunconnected:
        app.socketOut --> recorder.in;
        recorder.out --> dispatcher.in++;
        app.socketIn <-- dispatcher.out++;
        dispatcher.out++ --> delay.in;
        delay.out --> tcp.upperLayerIn;
        dispatcher.in++ <-- tcp.upperLayerOut;
        tcp.lowerLayerOut --> sink.in;
}

simple Ipv4
{
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input lowerLayerIn;
        output lowerLayerOut;
}

simple Ethernet
{
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input lowerLayerIn;
        output lowerLayerOut;
}

simple PacketQueue
{
    gates:
        input in;
        output out;
}

simple EthernetMac
{
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input lowerLayerIn;
        output lowerLayerOut;
}

module EthernetInterface
{
    parameters:
        @class(NetworkInterface);
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input physicalIn;
        output physicalOut;
    submodules:
        queue: PacketQueue;
        mac: EthernetMac;
    connections:
        upperLayerIn --> queue.in;
        queue.out --> mac.upperLayerIn;
        upperLayerOut <-- mac.upperLayerOut;
        mac.lowerLayerOut --> physicalOut;
        mac.lowerLayerIn <-- physicalIn;
}

// complete TCP/IP/Ethernet protocol stack without dispatchers
network TcpNetwork6
{
    submodules:
        app: TcpApp;
        recorder: PcapRecorder;
        delay: TcpProcessingDelay;
        tcp: Tcp;
        ip: Ipv4;
        ethernet: Ethernet;
        eth0: EthernetInterface;
        sink: PacketSink;
    connections allowunconnected:
        app.socketOut --> recorder.in;
        recorder.out --> delay.in;
        delay.out --> tcp.upperLayerIn;
        app.socketIn <-- tcp.upperLayerOut;
        tcp.lowerLayerOut --> ip.upperLayerIn;
        tcp.lowerLayerIn <-- ip.upperLayerOut;
        ip.lowerLayerOut --> ethernet.upperLayerIn;
        ip.lowerLayerIn <-- ethernet.upperLayerOut;
        ethernet.lowerLayerOut --> eth0.upperLayerIn;
        ethernet.lowerLayerIn <-- eth0.upperLayerOut;
        eth0.physicalOut --> sink.in;
}

// complete TCP/IP/Ethernet protocol stack with dispatchers
network TcpNetwork7
{
    submodules:
        app: TcpApp;
        td: Dispatcher;
        tcp: Tcp;
        nd: Dispatcher;
        ip: Ipv4;
        ld: Dispatcher;
        ethernet: Ethernet;
        id: Dispatcher;
        eth0: EthernetInterface;
        sink: PacketSink;
    connections allowunconnected:
        app.socketOut --> td.in++;
        app.socketIn <-- td.out++;
        td.out++ --> tcp.upperLayerIn;
        td.in++ <-- tcp.upperLayerOut;
        tcp.lowerLayerOut --> nd.in++;
        tcp.lowerLayerIn <-- nd.out++;
        nd.out++ --> ip.upperLayerIn;
        nd.in++ <-- ip.upperLayerOut;
        ip.lowerLayerOut --> ld.in++;
        ip.lowerLayerIn <-- ld.out++;
        ld.out++ --> ethernet.upperLayerIn;
        ld.in++ <-- ethernet.upperLayerOut;
        ethernet.lowerLayerOut --> id.in++;
        ethernet.lowerLayerIn <-- id.out++;
        id.out++ --> eth0.upperLayerIn;
        id.in++ <-- eth0.upperLayerOut;
        eth0.physicalOut --> sink.in;
}

simple Udp
{
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input lowerLayerIn;
        output lowerLayerOut;
}

simple Ipv6
{
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input lowerLayerIn;
        output lowerLayerOut;
}

simple Ppp
{
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input lowerLayerIn;
        output lowerLayerOut;
}

// complete TCP/IP/Ethernet protocol stack with dispatchers and multiple choices per level
network TcpNetwork8
{
    submodules:
        app: TcpApp;
        td: Dispatcher;
        tcp: Tcp;
        udp: Udp;
        nd: Dispatcher;
        ipv4: Ipv4;
        ipv6: Ipv6;
        ld: Dispatcher;
        ppp: Ppp;
        ethernet: Ethernet;
        id: Dispatcher;
        eth0: EthernetInterface;
        eth1: EthernetInterface;
        sink0: PacketSink;
        sink1: PacketSink;
    connections allowunconnected:
        app.socketOut --> td.in++;
        app.socketIn <-- td.out++;
        td.out++ --> tcp.upperLayerIn;
        td.in++ <-- tcp.upperLayerOut;
        td.out++ --> udp.upperLayerIn;
        td.in++ <-- udp.upperLayerOut;
        tcp.lowerLayerOut --> nd.in++;
        tcp.lowerLayerIn <-- nd.out++;
        udp.lowerLayerOut --> nd.in++;
        udp.lowerLayerIn <-- nd.out++;
        nd.out++ --> ipv4.upperLayerIn;
        nd.in++ <-- ipv4.upperLayerOut;
        nd.out++ --> ipv6.upperLayerIn;
        nd.in++ <-- ipv6.upperLayerOut;
        ipv4.lowerLayerOut --> ld.in++;
        ipv4.lowerLayerIn <-- ld.out++;
        ipv6.lowerLayerOut --> ld.in++;
        ipv6.lowerLayerIn <-- ld.out++;
        ld.out++ --> ethernet.upperLayerIn;
        ld.in++ <-- ethernet.upperLayerOut;
        ld.out++ --> ppp.upperLayerIn;
        ld.in++ <-- ppp.upperLayerOut;
        ethernet.lowerLayerOut --> id.in++;
        ethernet.lowerLayerIn <-- id.out++;
        ppp.lowerLayerOut --> id.in++;
        ppp.lowerLayerIn <-- id.out++;
        id.out++ --> eth0.upperLayerIn;
        id.in++ <-- eth0.upperLayerOut;
        id.out++ --> eth1.upperLayerIn;
        id.in++ <-- eth1.upperLayerOut;
        eth0.physicalOut --> sink0.in;
        eth1.physicalOut --> sink1.in;
}

network TcpNetwork9 extends TcpNetwork8
{
    connections allowunconnected:
        td.out++ --> nd.in++;
        td.in++ <-- nd.out++;
        nd.out++ --> ld.in++;
        nd.in++ <-- ld.out++;
        ld.out++ --> id.in++;
        ld.in++ <-- id.out++;
}
