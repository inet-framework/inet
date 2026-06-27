//
// Copyright (C) 2009 - today Brno University of Technology, Czech Republic
// Copyright (C) OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
// VRRPv2 (RFC 3768) ported from the ANSAINET project.
// Original authors: Petr Vitek, Vladimir Vesely (Brno University of Technology).
//

package inet.routing.vrrp;

import inet.common.SimpleModule;
import inet.routing.contract.IVrrp;

//
// Implements the Virtual Router Redundancy Protocol version 2 (RFC 3768) for
// IPv4. A single ~Vrrp module manages one or more virtual routers, each
// identified by an interface and a Virtual Router ID (VRID). Virtual routers
// are configured through the `configData` XML parameter:
//
// <VRRP>
//   <Interface name="eth0">
//     <Group id="1">
//       <IPAddress>192.168.1.254</IPAddress>
//       <Priority>200</Priority>
//       <Preempt>yes</Preempt>
//       <TimerAdvertise>1</TimerAdvertise>
//     </Group>
//   </Interface>
// </VRRP>
//
simple Vrrp extends SimpleModule like IVrrp
{
    parameters:
        @class(Vrrp);
        @display("i=block/network2");
        string interfaceTableModule = default("^.interfaceTable");
        string routingTableModule = default("^.ipv4.routingTable");
        string arpModule = default("^.ipv4.arp");
        xml configData = default(xml("<VRRP/>"));

        // per-virtual-router defaults (overridable from configData)
        int version = default(2);
        int priorityOwner = default(255);       // priority of the IP address owner
        int priorityDefault = default(100);      // default priority for a backup
        int timeToLive = default(255);           // IP TTL of advertisements (MUST be 255)
        double advertiseInterval @unit(s) = default(1s);
        bool preemptDefault = default(true);
        bool learnDefault = default(false);
        string multicastAddress = default("224.0.0.18");
        string virtualMacOui = default("00-00-5E-00-01-00"); // last octet replaced by VRID
        int arpType = default(2);                // 1 = ARP Request (RFC), 2 = ARP Reply (Cisco)
        double arpDelay @unit(s) = default(0.0000001s);

    gates:
        input ipIn @labels(Ipv4ControlInfo/up);
        output ipOut @labels(Ipv4ControlInfo/down);
}
