//
// Copyright (C) 2012 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.networklayer.ipv4;

//
// Module interface for IGMP modules.
//
// An IGMP module sends notifications about multicast group memberships in hosts,
// and processes these notifications in multicast routers.
//
// Currently there are 3 versions specified in RFC 1112 (IGMPv1), RFC 2236 (IGMPv2),
// and RFC 3376 (IGMPv3).
//
// IGMP packets are transported by the IP layer connected to the ipIn/ipOut
// gates. Some multicast routing protocols (e.g. DVMRP) also communitacate with
// IGMP messages. They must be connected to the routerIn/routerOut gates,
// the IGMP module will pass up the unrecognized IGMP packets.
//
// @see ~Igmpv2
//
moduleinterface IIgmp
{
    @display("i=block/cogwheel");

    gates:
        input ipIn @labels(Ipv4ControlInfo/up);
        output ipOut @labels(Ipv4ControlInfo/down);

        input routerIn @labels(Ipv4ControlInfo/up);
        output routerOut @labels(Ipv4ControlInfo/down);
}

