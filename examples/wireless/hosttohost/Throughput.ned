//
// Copyright (C) 2005 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

package inet.examples.wireless.hosttohost;

import inet.applications.ethernet.EtherAppClient;
import inet.linklayer.ieee80211.Ieee80211Interface;
import inet.mobility.single.CircleMobility;
import inet.mobility.static.StationaryMobility;
import inet.networklayer.common.InterfaceTable;
import inet.node.wireless.AccessPoint;
import inet.physicallayer.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.queueing.sink.PassivePacketSink;
import inet.visualizer.contract.IIntegratedVisualizer;

module ThroughputClient
{
    parameters:
        @display("i=device/wifilaptop");
        @networkNode();
        *.interfaceTableModule = "^.interfaceTable";
        wlan.radio.antenna.mobilityModule = default("^.^.^.mobility");
    gates:
        input radioIn @directIn;
    submodules:
        cli: EtherAppClient {
            parameters:
                @display("p=300,100");
        }
        wlan: Ieee80211Interface {
            parameters:
                @display("p=250,200;q=queue");
                mgmt.typename = "Ieee80211MgmtStaSimplified";
                agent.typename = "";
        }
        interfaceTable: InterfaceTable {
            parameters:
                @display("p=100,100;is=s");
        }
        mobility: CircleMobility {
            parameters:
                @display("p=100,200;is=s");
        }
    connections allowunconnected:
        wlan.radioIn <-- radioIn;
        cli.out --> wlan.upperLayerIn;
}

module ThroughputServer
{
    parameters:
        @display("i=device/wifilaptop");
        @networkNode();
        *.interfaceTableModule = "^.interfaceTable";
        wlan.radio.antenna.mobilityModule = default("^.^.^.mobility");
    gates:
        input radioIn @directIn;

    submodules:
        sink: PassivePacketSink {
            parameters:
                @display("p=300,100");
        }
        wlan: Ieee80211Interface {
            parameters:
                @display("p=250,200;q=queue");
                mgmt.typename = "Ieee80211MgmtStaSimplified";
                agent.typename = "";
        }
        interfaceTable: InterfaceTable {
            parameters:
                @display("p=100,100;is=s");
        }
        mobility: StationaryMobility {
            parameters:
                @display("p=100,200;is=s");
        }
    connections allowunconnected:
        wlan.radioIn <-- radioIn;
        sink.in <-- wlan.upperLayerOut;
}

network Throughput
{
    parameters:
        int numCli;
    submodules:
        visualizer: <default("IntegratedCanvasVisualizer")> like IIntegratedVisualizer if hasVisualizer() {
            parameters:
                @display("p=100,200;is=s");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            parameters:
                @display("p=100,100;is=s");
        }
        cliHost[numCli]: ThroughputClient {
            parameters:
                @display("r=,,#707070");
        }
        srvHost: ThroughputServer {
            parameters:
                @display("p=350,350;r=,,#707070");
        }
        ap: AccessPoint {
            parameters:
                @display("p=200,200;r=,,#707070");
                wlan[*].mgmt.typename = "Ieee80211MgmtApSimplified";
                wlan[*].agent.typename = "";
        }
}
