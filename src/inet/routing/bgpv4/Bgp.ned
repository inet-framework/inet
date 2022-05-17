//
// Copyright (C) 2010 Helene Lageber
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.routing.bgpv4;

import inet.routing.contract.IBgp;

//
// Implements BGP Version 4 routing protocol.
//
// The bgpConfig parameter can specify an XML file describing the
// configuration of all BGP routers and their interfaces in the network.
//
//# TODO config file!
//
simple Bgp like IBgp
{
    parameters:
        string interfaceTableModule;
        string routingTableModule;
        string ospfRoutingModule;
        volatile double startupTime @unit(s) = default(0s); // delay before starting BGP
        xml bgpConfig;
        double ExternalPeerStartDelayOffset @unit(s) = default(0.5s);
        bool redistributeRip = default(false);        // redistribute routes learned by RIP into BGP
        string redistributeOspf = default("");        // redistribute routes learned by OSPF into BGP (O IA E1 E2)
        bool redistributeInternal = default(false);   // redistribute BGP routes into OSPF (as external routes)
        bool nextHopSelf = default(false);
        int localPreference = default(100);
        bool connectedCheck = default(true);
        int ebgpMultihop = default(1);
        @display("i=block/network2");
        @selfMessageKinds(inet::bgp::BgpSelfMsgKind);
    gates:
        input socketIn @messageKinds(inet::TcpStatusInd);
        output socketOut @messageKinds(inet::TcpCommandCode);
}

