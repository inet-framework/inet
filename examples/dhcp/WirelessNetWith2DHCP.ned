//
// Copyright (C) 2016 OpenSim Ltd.
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

package inet.examples.dhcp;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ethernet.Eth100M;
import inet.node.inet.StandardHost;
import inet.node.inet.WirelessHost;
import inet.node.wireless.AccessPoint;
import inet.physicallayer.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.visualizer.contract.IIntegratedVisualizer;


network WirelessNetWith2DHCP
{
    parameters:
        int numHosts;
        *.eth[*].bitrate = 100 Mbps;
    submodules:
        visualizer: <default("IntegratedCanvasVisualizer")> like IIntegratedVisualizer if hasVisualizer() {
            @display("p=100,300;is=s");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            @display("p=100,200;is=s");
        }
        client: WirelessHost {
            @display("p=64,124");
        }
        ap1: AccessPoint {
            @display("p=64,238");
        }
        dhcpServer1: StandardHost {
            @display("p=64,307");
        }
        ap2: AccessPoint {
            @display("p=511,231");
        }
        dhcpServer2: StandardHost {
            @display("p=511,307");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=100,100;is=s");
        }
        server: StandardHost {
            @display("p=283,363");
        }
    connections:
        dhcpServer1.ethg++ <--> Eth100M <--> ap1.ethg++;
        dhcpServer2.ethg++ <--> Eth100M <--> ap2.ethg++;
        dhcpServer1.ethg++ <--> Eth100M <--> server.ethg++;
        dhcpServer2.ethg++ <--> Eth100M <--> server.ethg++;
}

