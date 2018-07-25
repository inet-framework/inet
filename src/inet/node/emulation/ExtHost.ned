//
// Copyright (C) 2018 OpenSim Ltd.
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

package inet.node.emulation;

import inet.emulation.common.tap.TapBridge;
import inet.emulation.common.tap.TapCfg;
import inet.linklayer.contract.IEthernetInterface;
import inet.linklayer.contract.IWirelessInterface;
import inet.mobility.contract.IMobility;
import inet.networklayer.common.InterfaceTable;


//
// An external host with Tap interfaces for connecting the simulation to real world.
//
module ExtHost
{
    parameters:
        int numEthInterfaces = default(0);  // minimum number of ethernet interfaces
        int numWlanInterfaces = default(0);
        @networkNode();
        @labels(node,ethernet-node);
        @display("i=device/pc2");
        mobility.typename = default(numWlanInterfaces > 0 ? "StationaryMobility" : "");
        mobility.subjectModule = default("^");
        eth[*].encap.typename = "EtherEncapDummy";
        wlan[*].llc.typename = default("Ieee80211Portal");
        wlan[*].radio.antenna.mobilityModule = default("^.^.^.mobility");
        *.interfaceTableModule = default(absPath(".interfaceTable"));
        *.mobilityModule = default(exists(mobility) ? absPath(".mobility") : "");
    gates:
        inout ethg[numEthInterfaces] @labels(EtherFrame-conn) @allowUnconnected;
        input radioIn[numWlanInterfaces] @directIn;
    submodules:
        interfaceTable: InterfaceTable {
            @display("p=53,73;is=s");
        }
        tap[sizeof(ethg)+numWlanInterfaces]: TapBridge {
            @display("p=232,104;q=txQueue");
        }
        tapCfg[sizeof(ethg)+numWlanInterfaces]: TapCfg {
            parameters:
                interfaceModule = index < sizeof(ethg) ? "^.eth["+string(index)+"]" : "^.wlan["+string(index - sizeof(ethg))+"]";
                @display("p=232,104;q=txQueue");
        }
        eth[sizeof(ethg)]: <default("EthernetInterface")> like IEthernetInterface {
            parameters:
                @display("p=138,234,row,150;q=txQueue");
        }
        wlan[numWlanInterfaces]: <default("Ieee80211Interface")> like IWirelessInterface {
            parameters:
                @display("p=302,234,row,150;q=queue");
        }
        mobility: <default("")> like IMobility if typename != "" {
            parameters:
                @display("p=53,162;is=s");
        }
    connections:
        for i=0..sizeof(ethg)-1 {
            tap[i].fromTap --> eth[i].upperLayerIn;
            tap[i].toTap <-- eth[i].upperLayerOut;
            eth[i].phys <--> { @display("m=s"); } <--> ethg[i];
        }
        for i=0..sizeof(radioIn)-1 {
            tap[sizeof(ethg) + i].fromTap --> wlan[i].upperLayerIn;
            tap[sizeof(ethg) + i].toTap <-- wlan[i].upperLayerOut;
            wlan[i].radioIn <-- { @display("m=s"); } <-- radioIn[i];
        }
}

