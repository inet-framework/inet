//
// Copyright (C) 2004 OpenSim Ltd.
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.networklayer.arp.ipv4;

import inet.networklayer.contract.IArp;

//
// Implements the Address Resolution Protocol for IPv4 and IEEE 802 6-byte
// MAC addresses.
//
// ARP packets are represented by the ~ArpPacket class.
//
// Communication Between ARP and IPv4 modules:
// IPv4 module call the ARP::getMACAddressFor(ipv4Address) method for get the MACAddress
// of next hop. If this method could not resolve an IPv4 address to MAC address, then IPv4
// uses the Arp::startAddressResolution() method.
// ARP emit a arpResolutionCompleted signal, when added/modified an entry in ARP table.
// ARP emit a arpResolutionFailed signal when an ARP request failed (ARP request sent
// retryCount times and retryTimeout elapsed).
//
// ARP resolution is used over <i>broadcast</i> interfaces only,
// that is, over interfaces that have the <tt>isBroadcast()</tt> flag set in
// <tt>NetworkInterface</tt> (see ~InterfaceTable). Since routing files
// (<tt>.irt</tt> or <tt>.mrt</tt> files, given as parameter to ~Ipv4RoutingTable)
// may modify the default interface configuration, you must take care that
// these files don't contain the word <tt>BROADCAST</tt> e.g. for PPP
// interfaces.
//
simple Arp like IArp
{
    parameters:
        string interfaceTableModule;   // The path to the InterfaceTable module
        string routingTableModule;
        double retryTimeout @unit(s) = default(1s);   // number seconds ARP waits between retries to resolve an IPv4 address
        int retryCount = default(3);                  // number of times ARP will attempt to resolve an IPv4 address
        double cacheTimeout @unit(s) = default(120s); // number seconds unused entries in the cache will time out
        string proxyArpInterfaces = default("*");     // list of interfaces that proxy ARP is enabled (all interfaces by default)
        @display("i=block/layer");
        @signal[arpRequestSent](type=inet::Packet);
        @signal[arpReplySent](type=inet::Packet);
        @signal[arpResolutionInitiated](type=inet::IArp::Notification);
        @signal[arpResolutionCompleted](type=inet::IArp::Notification);
        @signal[arpResolutionFailed](type=inet::IArp::Notification);
        @statistic[arpRequestSent](title="ARP request sent"; source=arpRequestSent; record=count,"sum(packetBytes)","vector(packetBytes)");
        @statistic[arpReplySent](title="ARP replies sent"; source=arpReplySent; record=count,"sum(packetBytes)","vector(packetBytes)");
        @statistic[arpResolutionInitiated](title="ARP initiated resolutions"; record=count);
        @statistic[arpResolutionCompleted](title="ARP completed resolutions"; record=count);
        @statistic[arpResolutionFailed](title="ARP failed resolutions"; record=count);
    gates:
        input netwIn @labels(ArpPacket+Ieee802Ctrl); // incoming ARP requests and replies
        output netwOut @labels(ArpPacket+Ieee802Ctrl,Ipv4Header+Ieee802Ctrl);  // outgoing ARP requests/replies, and datagrams with resolved next hop
        input ifIn @labels(ArpPacket,Ieee802Ctrl);
        output ifOut @labels(ArpPacket,Ieee802Ctrl);
}

