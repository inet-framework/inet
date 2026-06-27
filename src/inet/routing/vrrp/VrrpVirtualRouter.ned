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

//
// Represents a single VRRPv2 virtual router (one VRID on one interface). It runs
// the RFC 3768 state machine (Initialize / Backup / Master). Instances are
// created dynamically by the parent ~Vrrp module, which drives them via direct
// C++ calls (this module has no gates) and sends advertisements on their behalf.
//
simple VrrpVirtualRouter extends SimpleModule
{
    parameters:
        @class(VrrpVirtualRouter);
        @display("i=block/network2");
        @signal[vrState](type=long); // enum VrrpState
        @signal[sentAdvertisement](type=long);
        @signal[recvAdvertisement](type=long);
        @signal[sentArp](type=long);
        @statistic[vrState](title="state"; record=count,vector);
        @statistic[sentAdvertisement](title="advertisements sent"; record=count,vector);
        @statistic[recvAdvertisement](title="advertisements received"; record=count,vector);
        @statistic[sentArp](title="gratuitous ARPs sent"; record=count,vector);

        int vrid;                                  // Virtual Router ID (1..255)
        int interfaceId;                           // ID of the interface this VR runs on
        string virtualIps;                         // space-separated; the first one is the primary/group address
        int priority = default(100);               // 1..254 (0 reserved, 255 = owner)
        bool preempt = default(true);
        double preemptDelay @unit(s) = default(0s);
        double advertiseInterval @unit(s) = default(1s);
        bool learn = default(false);
        string description = default("");

        // defaults propagated from the parent Vrrp module
        int version = default(2);
        int priorityOwner = default(255);
        int timeToLive = default(255);             // advertisements MUST use TTL 255
        string multicastAddress = default("224.0.0.18");
        string virtualMacOui = default("00-00-5E-00-01-00"); // last octet replaced by VRID
        int arpType = default(2);                  // 1 = ARP Request (RFC), 2 = ARP Reply (Cisco)
        double arpDelay @unit(s) = default(0.0000001s);
}
