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
import inet.linklayer.contract.IExternalNic;
import inet.linklayer.contract.ITunNic;
import inet.linklayer.contract.IWiredNic;
import inet.linklayer.contract.IWirelessNic;
import inet.linklayer.loopback.LoopbackInterface;
import inet.networklayer.common.InterfaceTable;

module LinkLayerNodeBase extends NodeBase
{
    parameters:
        bool hasPcapRecorder = default(false);
        int numLoInterfaces = default(1);
        int numWlanInterfaces = default(0);
        int numExtInterfaces = default(0);
        int numTunInterfaces = default(0);
        mobilityType = default(numWlanInterfaces > 0 ? "StationaryMobility" : "");
        *.interfaceTableModule = default(absPath(".interfaceTable"));
        @figure[linkLayer](type=rectangle; pos=250,600; size=680,200; fillColor=#0000ff; cornerRadius=12; fillOpacity=0.1);
        @figure[linkLayer.title](type=text; pos=800,610; text="link layer");

    gates:
        input radioIn[numWlanInterfaces] @directIn;
        inout pppg[] @labels(PPPFrame-conn);
        inout ethg[] @labels(EtherFrame-conn);

    submodules:
        interfaceTable: InterfaceTable {
            parameters:
                @display("p=100,600;is=s");
        }
        pcapRecorder: PcapRecorder if hasPcapRecorder {
            parameters:
                @display("p=100,700;is=s");
        }
        lo[numLoInterfaces]: LoopbackInterface {
            parameters:
                @display("p=300,700,row,50;");
        }
        // TODO: move wlan interfaces after eth interfaces, but it changes IP address assignment and breaks examples/inet/configurator/complex.ini
        wlan[numWlanInterfaces]: <default("Ieee80211Nic")> like IWirelessNic {
            parameters:
                @display("p=600,700,row,50;q=queue");
        }
        ppp[sizeof(pppg)]: <default("PPPInterface")> like IWiredNic {
            parameters:
                @display("p=400,700,row,50;q=txQueue");
        }
        eth[sizeof(ethg)]: <default("EthernetInterface")> like IWiredNic {
            parameters:
                @display("p=500,700,row,50;q=txQueue");
        }
        ext[numExtInterfaces]: <default("ExtInterface")> like IExternalNic {
            parameters:
                @display("p=700,700,row,50;q=txQueue");
        }
        tun[numTunInterfaces]: <default("TunInterface")> like ITunNic {
            parameters:
                @display("p=800,700,row,50;q=txQueue");
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
