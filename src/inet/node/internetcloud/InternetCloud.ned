//
// Copyright (C) 2012 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.node.internetcloud;

import inet.networklayer.internetcloud.ICloudDelayer;
import inet.node.base.NetworkLayerNodeBase;


//
// InternetCloud models a large network such as part of the Internet or a corporate
// backbone network with realistic delay, packet loss, and datarate characteristics.
// It abstracts away the internal structure of the network while preserving the
// end-to-end behavior that applications would experience.
//
// Key features:
// - Configurable delay, packet loss, and datarate for different paths
// - Preservation of packet order within each flow
// - Support for IPv4, IPv6, and generic network protocols
// - Replaceable delayer module with customizable behavior
// - Ability to model complex network effects without detailed topology
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

