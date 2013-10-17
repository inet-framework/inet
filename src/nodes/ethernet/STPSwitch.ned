//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

package inet.nodes.ethernet;

import inet.base.NotificationBoard;
import inet.networklayer.common.InterfaceTable;
import inet.linklayer.IWiredNic;
import inet.linklayer.ethernet.switch.MACAddressTable;
import inet.status.NodeStatus;
import inet.linklayer.ieee8021d.stp.SpanningTree;
import inet.linklayer.ethernet.switch.Relay.IEEE8021DRelay;
import inet.linklayer.configurator.L2NodeConfigurator;

module STPSwitch
{
    parameters:
        @node();
        @display("i=device/router;bgb=386,346;bgl=2");
        bool hasStatus = default(false);
        eth[*].encapType = "EtherEncapDummy";
    gates:
        inout ethg[];
    submodules:
        status: NodeStatus if hasStatus {
            @display("p=50,50;is=s");
        }
        l2NodeConfigurator: L2NodeConfigurator {
            @display("p=50,181");
        }
        interfaceTable: InterfaceTable {
            @display("p=50,92;is=s");
        }

        notificationBoard: NotificationBoard {
            parameters:
                @display("p=50,135;is=s");
        }

        relayUnit: IEEE8021DRelay {
            parameters:
                @display("p=199,50;is=l");
            gates:
                ifIn[sizeof(ethg)];
                ifOut[sizeof(ethg)];
        }

        macTable: MACAddressTable {
            parameters:
                @display("p=316,50");
        }

        eth[sizeof(ethg)]: <default("EthernetInterface")> like IWiredNic {
            parameters:
                mac.promiscuous = true;
                @display("p=199,281,row;q=txQueue");
        }

        stp: SpanningTree {
            @display("p=103,50");
        }

    connections:
        for i=0..sizeof(ethg)-1 {
            eth[i].upperLayerIn <-- relayUnit.ifOut++;
            eth[i].upperLayerOut --> relayUnit.ifIn++;
            eth[i].phys <--> ethg[i];
        }

        stp.STPGate <--> relayUnit.STPGate; 
}
