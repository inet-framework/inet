//
// Copyright (C) 2003 Andras Varga; CTIE, Monash University, Australia
//               2010 Zoltan Bojthe
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

package inet.node.ethernet;

import inet.common.lifecycle.NodeStatus;
import inet.linklayer.IMACAddressTable;
import inet.linklayer.IMACRelayUnit;
import inet.linklayer.ISpanningTree;
import inet.linklayer.IWiredNic;
import inet.linklayer.configurator.L2NodeConfigurator;
import inet.networklayer.common.InterfaceTable;


//
// Model of an Ethernet switch.
//
// The duplexChannel attributes of the MACs must be set according to the
// medium connected to the port; if collisions are possible (it's a bus or hub)
// it must be set to false, otherwise it can be set to true.
// By default used half duples CSMA/CD mac
//
//
module EtherSwitch
{
    parameters:
        @node();
        @labels(node,ethernet-node);
        @display("i=device/switch");
        bool hasStatus = default(false);
        bool csmacdSupport = default(true);  // by default use CSMA/CD
        string macType = default(csmacdSupport ? "EtherMAC" : "EtherMACFullDuplex"); // ~EtherMAC or ~EtherMACFullDuplex
        string spanningTreeProtocol = default("");
        string relayUnitType = default(firstAvailable("Ieee8021dRelay","MACRelayUnit")); // type of the ~IMACRelayUnit;
        string macTableType = default("MACAddressTable"); // type of the ~IMACAddressTable
        eth[*].encapType = "EtherEncapDummy";
        eth[*].csmacdSupport = csmacdSupport;
        eth[*].macType = macType;
        *.interfaceTableModule = default(absPath(".interfaceTable"));
    gates:
        inout ethg[] @labels(EtherFrame-conn);
    submodules:
        status: NodeStatus if hasStatus {
            @display("p=73,111;is=s");
        }
        l2NodeConfigurator: L2NodeConfigurator if spanningTreeProtocol != "" {
            @display("p=73,208");
        }
        interfaceTable: InterfaceTable {
            @display("p=73,65;is=s");
        }
        macTable: <macTableType> like IMACAddressTable {
            @display("p=73,19;is=s");
        }
        relayUnit: <relayUnitType> like IMACRelayUnit {
            parameters:
                @display("p=263,50;is=m");
            gates:
                ifIn[sizeof(ethg)];
                ifOut[sizeof(ethg)];
        }
        stp: <spanningTreeProtocol> like ISpanningTree if spanningTreeProtocol != "" {
            @display("p=168,50");
        }
        eth[sizeof(ethg)]: <default("EthernetInterface")> like IWiredNic {
            parameters:
                mac.promiscuous = true;
                @display("p=263,150,row;q=txQueue");
        }
    connections:
        for i=0..sizeof(ethg)-1 {
            eth[i].upperLayerIn <-- relayUnit.ifOut++;
            eth[i].upperLayerOut --> relayUnit.ifIn++;
            eth[i].phys <--> ethg[i];
        }

        if spanningTreeProtocol != "" {
            stp.relayIn <-- relayUnit.stpOut;
            stp.relayOut --> relayUnit.stpIn;
        }
}
