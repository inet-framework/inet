//
// Copyright (C) OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

package inet.node.base;

import inet.common.packet.recorder.PcapRecorder;
import inet.linklayer.contract.IEthernetInterface;
import inet.linklayer.contract.IExternalInterface;
import inet.linklayer.contract.ILoopbackInterface;
import inet.linklayer.contract.IPppInterface;
import inet.linklayer.contract.ITunnelInterface;
import inet.linklayer.contract.IWirelessInterface;
import inet.linklayer.contract.IVlanInterface;
import inet.networklayer.common.InterfaceTable;

module LinkLayerNodeBase extends NodeBase
{
    parameters:
        int numPcapRecorders = default(0);
        int numLoInterfaces = default(1);
        int numWlanInterfaces = default(0);
        int numEthInterfaces = default(0);  // minimum number of ethernet interfaces
        int numPppInterfaces = default(0);  // minimum number of PPP interfaces
        int numTunInterfaces = default(0);
        int numVlanInterfaces = default(0);
        mobility.typename = default(numWlanInterfaces > 0 ? "StationaryMobility" : "");
        *.interfaceTableModule = default(absPath(".interfaceTable"));
        wlan[*].radio.antenna.mobilityModule = default("^.^.^.mobility");
        @figure[linkLayer](type=rectangle; pos=250,456; size=1000,210; fillColor=#0000ff; lineColor=#808080; cornerRadius=5; fillOpacity=0.1);
        @figure[linkLayer.title](type=text; pos=1245,461; anchor=ne; text="link layer");

    gates:
        input radioIn[numWlanInterfaces] @directIn;
        inout pppg[numPppInterfaces] @labels(PppFrame-conn) @allowUnconnected;
        inout ethg[numEthInterfaces] @labels(EtherFrame-conn) @allowUnconnected;

    submodules:
        pcapRecorder[numPcapRecorders]: PcapRecorder {
            parameters:
                @display("p=125,560;is=s");
        }
        interfaceTable: InterfaceTable {
            parameters:
                @display("p=125,240;is=s");
        }
        lo[numLoInterfaces]: <default("LoopbackInterface")> like ILoopbackInterface {
            parameters:
                @display("p=750,526,row,150");
        }
        // TODO: move wlan interfaces after eth interfaces, but it changes IP address assignment and breaks examples/inet/configurator/complex.ini
        wlan[numWlanInterfaces]: <default("Ieee80211Interface")> like IWirelessInterface {
            parameters:
                @display("p=375,616,row,150;q=queue");
        }
        ppp[sizeof(pppg)]: <default("PppInterface")> like IPppInterface {
            parameters:
                @display("p=300,526,row,150;q=txQueue");
        }
        eth[sizeof(ethg)]: <default("EthernetInterface")> like IEthernetInterface {
            parameters:
                @display("p=900,526,row,150;q=txQueue");
        }
        tun[numTunInterfaces]: <default("TunInterface")> like ITunnelInterface {
            parameters:
                @display("p=975,616,row,150;q=txQueue");
        }
        vlan[numVlanInterfaces]: <default("VlanInterface")> like IVlanInterface {
            parameters:
                @display("p=975,616,row,150;q=txQueue");
        }

    connections allowunconnected:
        for i=0..sizeof(radioIn)-1 {
            radioIn[i] --> { @display("m=s"); } --> wlan[i].radioIn;
        }

        for i=0..sizeof(ethg)-1 {
            ethg[i] <--> { @display("m=s"); } <--> eth[i].phys;
        }

        for i=0..sizeof(pppg)-1 {
            pppg[i] <--> { @display("m=s"); } <--> ppp[i].phys;
        }
}

