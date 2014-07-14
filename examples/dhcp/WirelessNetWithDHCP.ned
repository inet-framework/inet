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

import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;
import inet.node.ethernet.Eth100M;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import inet.node.inet.WirelessHost;
import inet.node.wireless.AccessPoint;
import inet.physicallayer.ieee80211.Ieee80211RadioMedium;


network WirelessNetWithDHCP
{
    @display("bgb=590,407;bgl=2");
    int numHosts;
    **.mgmt.numChannels = 5;
    submodules:
        radioMedium: Ieee80211RadioMedium {
            @display("p=64,34");
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
        configurator: IPv4NetworkConfigurator {
            parameters:
                @display("p=180,35");
        }
        remote: StandardHost {
            @display("p=540,204");
        }
    connections:
        gateway.ethg++ <--> Eth100M <--> ap.ethg++;
        gateway.ethg++ <--> Eth100M <--> remote.ethg++;
        dhcpServer.ethg++ <--> Eth100M <--> ap.ethg++;
}
