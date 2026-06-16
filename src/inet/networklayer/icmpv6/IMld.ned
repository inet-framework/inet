//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.networklayer.icmpv6;

//
// Module interface for MLD modules.
//
// An MLD module sends notifications about IPv6 multicast group memberships
// in hosts, and processes these in multicast routers. MLD is the IPv6
// equivalent of IGMP; MLDv1 (RFC 2710) parallels IGMPv2 (RFC 2236).
//
// MLD packets are transported as ICMPv6 messages by the IPv6 layer
// connected to the `ipIn`/`ipOut` gates (which attach to the `lp`
// MessageDispatcher in ~Ipv6NetworkLayer).
//
// @see ~Mldv1
//
moduleinterface IMld
{
    @display("i=block/cogwheel");

    gates:
        input ipIn @labels(Ipv6ControlInfo/up);
        output ipOut @labels(Ipv6ControlInfo/down);
}
