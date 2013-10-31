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

package inet.nodes.ethernet;

import inet.base.NotificationBoard;
import inet.linklayer.IMACRelayUnit;
import inet.linklayer.IWiredNic;
import inet.networklayer.common.InterfaceTable;
import inet.status.NodeStatus;
import inet.linklayer.ieee8021d.stp.STP;
import inet.linklayer.ieee8021d.stp.STP;
import inet.linklayer.ethernet.switch.MACAddressTable;
import inet.linklayer.configurator.L2NodeConfigurator;
import inet.linklayer.ieee8021d.rstp.RSTP;
import inet.linklayer.ethernet.switch.Relay.IEEE8021DRelay;
import inet.linklayer.ieee8021d.ISpanningTree;

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
        @display("i=device/switch;bgb=251,213");
        bool hasStatus = default(false);
        bool csmacdSupport = default(true);  // by default use CSMA/CD
        string macType = default(csmacdSupport ? "EtherMAC" : "EtherMACFullDuplex"); // ~EtherMAC or ~EtherMACFullDuplex
        string spanningTreeProtocolVersion = default("STP");
        eth[*].encapType = "EtherEncapDummy";
        eth[*].csmacdSupport = csmacdSupport;
        eth[*].macType = macType;
    gates:
        inout ethg[] @labels(EtherFrame-conn);
    submodules:
        status: NodeStatus if hasStatus {
            @display("p=50,58;is=s");
        }
        l2NodeConfigurator: L2NodeConfigurator {
            @display("p=50,181");
        }
        interfaceTable: InterfaceTable {
            @display("p=50,100;is=s");
        }
        notificationBoard: NotificationBoard {
            parameters:
                @display("p=50,150;is=s");
        }
        macTable: MACAddressTable {
            @display("p=50,19;is=s");
        }
        relayUnit: IEEE8021DRelay {
            parameters:
                @display("p=198,50;is=m");
            gates:
                ifIn[sizeof(ethg)];
                ifOut[sizeof(ethg)];
        }
        stp: <spanningTreeProtocolVersion> like ISpanningTree if spanningTreeProtocolVersion != "" {
            @display("p=125,50");
        }
        eth[sizeof(ethg)]: <default("EthernetInterface")> like IWiredNic {
            parameters:
                mac.promiscuous = true;
                @display("p=198,150,row;q=txQueue");
        }
    connections:
        for i=0..sizeof(ethg)-1 {
            eth[i].upperLayerIn <-- relayUnit.ifOut++;
            eth[i].upperLayerOut --> relayUnit.ifIn++;
            eth[i].phys <--> ethg[i];
        }
        
        if spanningTreeProtocolVersion != "" {
        	stp.STPGate <--> relayUnit.STPGate;
        }
}
