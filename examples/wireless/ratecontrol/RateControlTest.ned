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

package inet.examples.wireless.ratecontrol;

import inet.applications.ethernet.EtherAppClient;
import inet.applications.ethernet.EtherAppServer;
import inet.environment.common.PhysicalEnvironment;
import inet.linklayer.ieee80211.Ieee80211Interface;
import inet.mobility.single.RectangleMobility;
import inet.networklayer.common.InterfaceTable;
import inet.physicallayer.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;


module SinkClient
{
    parameters:
        @networkNode();
        @labels(node);
        @display("i=device/cellphone");
        *.interfaceTableModule = default(absPath(".interfaceTable"));
        wlan.radio.antenna.mobilityModule = default("^.^.^.mobility");
    gates:
        input radioIn @directIn;
    submodules:
        interfaceTable: InterfaceTable {
            @display("p=100,100;is=s");
        }
        sink: EtherAppServer {
            parameters:
                @display("p=300,100");
        }
        wlan: Ieee80211Interface {
            parameters:
                @display("p=250,200;q=queue");
                mgmt.typename = "Ieee80211MgmtAdhoc";
                agent.typename = "";
        }
        mobility: RectangleMobility {
            parameters:
                @display("p=100,200;is=s");
        }
    connections allowunconnected:
        radioIn --> { @display("m=s"); } --> wlan.radioIn;
        wlan.upperLayerOut --> sink.in;
        sink.out --> wlan.upperLayerIn;
}

module Client
{
    parameters:
        @display("i=device/wifilaptop");
        @networkNode();
        *.interfaceTableModule = default(absPath(".interfaceTable"));
        wlan.radio.antenna.mobilityModule = default("^.^.^.mobility");
    gates:
        input radioIn @directIn;

    submodules:
        interfaceTable: InterfaceTable {
            @display("p=100,100;is=s");
        }
        cli: EtherAppClient {
            parameters:
                @display("p=300,100");
        }
        wlan: Ieee80211Interface {
            parameters:
                @display("p=250,200;q=queue");
                mgmt.typename = "Ieee80211MgmtAdhoc";
                agent.typename = "";
        }
        mobility: RectangleMobility {
            parameters:
                @display("p=100,200;is=s");
        }
    connections allowunconnected:
        wlan.radioIn <-- radioIn;
        cli.out --> wlan.upperLayerIn;
}

network RateControlTest
{
    parameters:
        int numCli;
        @display("bgb=1300,460");
    submodules:
        cliHost[numCli]: Client {
            parameters:
                @display("r=,,#707070");
        }
        sinkClient: SinkClient {
            parameters:
                @display("p=200,200;r=,,#707070");
        }
        physicalEnvironment: PhysicalEnvironment {
            parameters:
                @display("p=100,100;is=s");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            parameters:
                @display("p=100,200;is=s");
        }
}

