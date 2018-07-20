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

import inet.common.MessageDispatcher;
import inet.common.lifecycle.NodeStatus;
import inet.linklayer.configurator.L2NodeConfigurator;
import inet.linklayer.contract.IEthernetInterface;
import inet.linklayer.contract.IMacAddressTable;
import inet.linklayer.contract.IMacRelayUnit;
import inet.linklayer.contract.ISpanningTree;
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
        @networkNode();
        @labels(node,ethernet-node);
        @display("i=device/switch");
        bool hasStatus = default(false);
        bool csmacdSupport = default(true);  // by default use CSMA/CD --FIXME remove
        string spanningTreeProtocol = default("");
        eth[*].encap.typename = "EtherEncapDummy";
        eth[*].csmacdSupport = csmacdSupport;
        eth[*].mac.typename = default(csmacdSupport ? "EtherMac" : "EtherMacFullDuplex");
        *.interfaceTableModule = default(absPath(".interfaceTable"));
        *.macTableModule = default(absPath(".macTable"));
    gates:
        inout ethg[] @labels(EtherFrame-conn);
    submodules:
        status: NodeStatus if hasStatus {
            @display("p=100,400;is=s");
        }
        l2NodeConfigurator: L2NodeConfigurator if spanningTreeProtocol != "" {
            @display("p=100,300;is=s");
        }
        interfaceTable: InterfaceTable {
            @display("p=100,200;is=s");
        }
        macTable: <default("MacAddressTable")> like IMacAddressTable {
            @display("p=100,100;is=s");
        }
        relayUnit: <default(firstAvailable("Ieee8021dRelay","MacRelayUnit"))> like IMacRelayUnit {
            @display("p=400,100;is=m");
        }
        stp: <spanningTreeProtocol> like ISpanningTree if spanningTreeProtocol != "" {
            @display("p=250,100");
        }
        nl: MessageDispatcher {
            parameters:
                @display("p=800,200;b=1200,5");
        }
        eth[sizeof(ethg)]: <default("EthernetInterface")> like IEthernetInterface {
            parameters:
                mac.promiscuous = true;
                @display("p=250,300,row,150;q=txQueue");
        }
    connections:
        relayUnit.ifOut --> nl.in++;
        relayUnit.ifIn <-- nl.out++;
        for i=0..sizeof(ethg)-1 {
            nl.out++ --> eth[i].upperLayerIn;
            nl.in++ <-- eth[i].upperLayerOut;
            eth[i].phys <--> { @display("m=s"); } <--> ethg[i];
        }

        if spanningTreeProtocol != "" {
            stp.relayIn <-- relayUnit.stpOut;
            stp.relayOut --> relayUnit.stpIn;
        }
}
