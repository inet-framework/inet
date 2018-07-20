//
// Copyright (C) 2010 Juan-Carlos Maureira
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
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import inet.node.inet.WirelessHost;
import inet.node.wireless.AccessPoint;
import inet.physicallayer.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;


network WirelessNetWithDHCP
{
    int numHosts;
    **.mgmt.numChannels = 5;
    submodules:
        radioMedium: Ieee80211ScalarRadioMedium {
            @display("p=100,200;is=s");
        }
        gateway: Router {
            @display("p=365,35");
        }
        client[numHosts]: WirelessHost {
            @display("r=,,#707070;p=245,285");
        }
        ap: AccessPoint {
            @display("p=126,98");
        }
        dhcpServer: StandardHost {
            @display("p=64,307");
        }
        configurator: Ipv4NetworkConfigurator {
            parameters:
                @display("p=100,100;is=s");
        }
        remote: StandardHost {
            @display("p=540,204");
        }
    connections:
        gateway.ethg++ <--> Eth100M <--> ap.ethg++;
        gateway.ethg++ <--> Eth100M <--> remote.ethg++;
        dhcpServer.ethg++ <--> Eth100M <--> ap.ethg++;
}
