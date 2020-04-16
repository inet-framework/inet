//
// Copyright (C) 2012 OpenSim Ltd
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//
// @author Zoltan Bojthe
//

package inet.node.internetcloud;

import inet.common.MessageDispatcher;
import inet.common.packet.recorder.PcapRecorder;
import inet.linklayer.contract.IWiredInterface;
import inet.networklayer.arp.ipv4.Arp;
import inet.networklayer.common.InterfaceTable;
import inet.networklayer.configurator.ipv4.Ipv4NodeConfigurator;
import inet.networklayer.internetcloud.ICloudDelayer;
import inet.networklayer.ipv4.Icmp;
import inet.networklayer.ipv4.Ipv4;
import inet.networklayer.ipv4.Ipv4RoutingTable;
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

