//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.node.inet;

import inet.applications.contract.IApp;
import inet.node.base.ApplicationLayerNodeBase;
import inet.routing.contract.IBgp;
import inet.routing.contract.IOspf;
import inet.routing.contract.IPim;
import inet.routing.contract.IEigrp;

//
// A network router device with support for multiple routing protocols
// and network interfaces. Provides the foundation for simulating various types of
// routers in both wired and wireless networks.
//
// The Router module extends ApplicationLayerNodeBase with routing protocol support,
// including OSPF, BGP, RIP, PIM, and EIGRP. It has packet forwarding enabled by default
// and can be configured with various interface types.
//
// @see ~StandardHost
//
module Router extends ApplicationLayerNodeBase
{
    parameters:
        @display("i=abstract/router");
        @figure[submodules];
        forwarding = true;
        bool hasEigrp = default(false);
        bool hasOspf = default(false);
        bool hasRip = default(false);
        bool hasBgp = default(false);
        bool hasPim = default(false);
        bool hasDhcp = default(false);
        hasUdp = default(hasRip || hasDhcp);
        hasTcp = default(hasBgp);
        *.routingTableModule = default("^.ipv4.routingTable");

    submodules:
        ospf: <default("Ospfv2")> like IOspf if hasOspf {
            parameters:
                @display("p=975,226");
        }
        bgp: <"Bgp"> like IBgp if hasBgp {
            parameters:
                ospfRoutingModule = default(parent.hasOspf ? "^.ospf" : "");
                @display("p=825,76");
        }
        rip: <"Rip"> like IApp if hasRip {
            parameters:
                @display("p=975,76");
        }
        pim: <"Pim"> like IPim if hasPim {
            parameters:
                @display("p=825,226");
        }
        dhcp: <"DhcpServer"> like IApp if hasDhcp {
            parameters:
                @display("p=1125,76");
        }
        eigrp: <"EigrpProcessDS"> like IEigrp if hasEigrp {
            parameters:
                enableIPv6 = parent.hasIpv6;
                enableIPv4 = parent.hasIpv4;
                @display("p=1113,225");
        }

    connections allowunconnected:
        ospf.ipOut --> tn.in++ if hasOspf;
        ospf.ipIn <-- tn.out++ if hasOspf;

        eigrp.ipOut --> tn.in++ if hasEigrp;
        eigrp.ipIn <-- tn.out++ if hasEigrp;

        bgp.socketOut --> at.in++ if hasBgp;
        bgp.socketIn <-- at.out++ if hasBgp;

        rip.socketOut --> at.in++ if hasRip;
        rip.socketIn <-- at.out++ if hasRip;

        pim.networkLayerOut --> tn.in++ if hasPim;
        pim.networkLayerIn <-- tn.out++ if hasPim;

        dhcp.socketOut --> at.in++ if hasDhcp;
        dhcp.socketIn <-- at.out++ if hasDhcp;
}

