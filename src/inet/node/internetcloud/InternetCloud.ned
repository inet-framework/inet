//
// Copyright (C) 2012 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.node.internetcloud;

import inet.networklayer.internetcloud.ICloudDelayer;
import inet.node.base.NetworkLayerNodeBase;


//
// This module is an IPv4 router that can delay or drop packets (while retaining their order)
// based on which interface card the packet arrived on and on which interface
// it is leaving the cloud. The delayer module is replaceable.
//
// By default the delayer module is ~MatrixCloudDelayer which lets you configure
// the delay, drop and datarate parameters in an XML file. Packet flows, as defined
// by incoming and outgoing interface pairs, are independent of each other.
//
// The ~InternetCloud module can be used only to model the delay between two hops, but
// it is possible to build more complex networks using several ~InternetCloud modules.
//
// @see ~MatrixCloudDelayer
//
module InternetCloud extends NetworkLayerNodeBase
{
    parameters:
        numLoInterfaces = 0;
        numTunInterfaces = 0;
        forwarding = true;
        @display("i=misc/cloud");
        @figure[submodules];
        *.interfaceTableModule = default(absPath(".interfaceTable"));
    submodules:
        ipv4Delayer: <default("MatrixCloudDelayer")> like ICloudDelayer if hasIpv4 {
            parameters:
                networkProtocolModule = "^.ipv4.ip";
                @display("p=825,376");
        }
        ipv6Delayer: <default("MatrixCloudDelayer")> like ICloudDelayer if hasIpv6 {
            parameters:
                networkProtocolModule = "^.ipv6.ipv6";
                @display("p=975,376");
        }
        gnDelayer: <default("MatrixCloudDelayer")> like ICloudDelayer if hasGn {
            parameters:
                networkProtocolModule = "^.generic.nextHop";
                @display("p=1125,376");
        }
    connections allowunconnected:
}

