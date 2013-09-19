//
// Copyright (C) 2011 Juan Luis Garrote Molinero
// Copyright (C) 2013 Zsolt Prontvai
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

package inet.nodes.rstp;

//Ethernet switch with RSTP capability

import inet.base.NotificationBoard;
import inet.networklayer.common.InterfaceTable;
import inet.linklayer.IWiredNic;
import inet.linklayer.ethernet.switch.MACAddressTable;
import inet.linklayer.rstp.RSTP;
import inet.linklayer.rstp.RelayRSTP;
import inet.linklayer.rstp.PortFiltRSTP;

module RSTPSwitch
{
    parameters:
        @node();
        @display("i=device/router;bgb=386,346;bgl=2");
        eth[*].encapType = "EtherEncapDummy";
    gates:
        inout ethg[];
    submodules:
        interfaceTable: InterfaceTable {
            @display("p=50,92;is=s");
        }
        notificationBoard: NotificationBoard {
            parameters:
                @display("p=50,150;is=s");
        }
        relay: RelayRSTP {
            parameters:
                @display("p=199,50");
            gates:
                GatesIn[sizeof(ethg)];
                GatesOut[sizeof(ethg)];
                RSTPGate;
        }
        macTable: MACAddressTable {
            parameters:
                @display("p=316,50");
        }

        PortFilt[sizeof(ethg)]: PortFiltRSTP {
            parameters:
                @display("p=199,150,row");
            gates:
                GatesIn[];
                GatesOut[];

        }
        
        eth[sizeof(ethg)]: <default("EthernetInterface")> like IWiredNic {
            parameters:
                mac.promiscuous = true;
                @display("p=199,281,row;q=txQueue");
        }
        
        rstp: RSTP {
            @display("p=127,50");
            gates:
                RSTPPort;
        }												// RSTP protocol managing.

    connections:
        for i=0..sizeof(ethg)-1 {
            ethg[i] <--> eth[i].phys;
            PortFilt[i].GatesOut[0] --> eth[i].upperLayerIn;
            PortFilt[i].GatesIn[0] <-- eth[i].upperLayerOut;
            PortFilt[i].GatesIn[1] <-- relay.GatesOut[i];
            PortFilt[i].GatesOut[1] --> relay.GatesIn[i];
        }

        rstp.RSTPPort <--> relay.RSTPGate;
}
