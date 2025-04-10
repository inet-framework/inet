//
// Copyright (C) 2004 OpenSim Ltd.
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.networklayer.arp.ipv4;

import inet.common.SimpleModule;
import inet.networklayer.contract.IArp;

//
// Implements the Address Resolution Protocol for IPv4 and IEEE 802 6-byte
// MAC addresses.
//
// ARP packets are represented by the ~ArpPacket class.
//
// Communication Between ARP and IPv4 modules:
// IPv4 module calls the `ARP::getMACAddressFor(ipv4Address)` method to get the MACAddress
// of the next hop. If this method could not resolve an IPv4 address to a MAC address, then IPv4
// uses the `Arp::startAddressResolution()` method.
// ARP emits an `arpResolutionCompleted` signal when it adds/modifies an entry in the ARP table.
// ARP emits an `arpResolutionFailed` signal when an ARP request fails (ARP request sent
// `retryCount` times and `retryTimeout` elapsed).
//
// ARP resolution is used on <i>broadcast</i> interfaces only,
// that is, on interfaces that have the `isBroadcast()` flag set in
// `NetworkInterface` (see ~InterfaceTable). Since routing files
// (`.irt` or `.mrt` files, given as parameters to ~Ipv4RoutingTable)
// may modify the default interface configuration, you must ensure that
// these files don't contain the word `BROADCAST` e.g. for PPP
// interfaces.
//
simple Arp extends SimpleModule like IArp
{
    parameters:
        @class(Arp);
        string interfaceTableModule;   // The path to the InterfaceTable module
        string routingTableModule;
        double retryTimeout @unit(s) = default(1s);   // Number of seconds ARP waits between retries to resolve an IPv4 address
        int retryCount = default(3);                  // Number of times ARP will attempt to resolve an IPv4 address
        double cacheTimeout @unit(s) = default(120s); // Number of seconds unused entries in the cache will time out
        string proxyArpInterfaces = default("*");     // List of interfaces that proxy ARP is enabled on (all interfaces by default)
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
        input netwIn @labels(ArpPacket+Ieee802Ctrl); // Incoming ARP requests and replies
        output netwOut @labels(ArpPacket+Ieee802Ctrl,Ipv4Header+Ieee802Ctrl);  // Outgoing ARP requests/replies, and datagrams with resolved next hop
        input ifIn @labels(ArpPacket,Ieee802Ctrl);
        output ifOut @labels(ArpPacket,Ieee802Ctrl);
}

