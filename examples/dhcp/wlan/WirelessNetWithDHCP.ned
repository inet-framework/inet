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

package inet.examples.dhcp.wlan;
import inet.nodes.ethernet.Eth100M;
import inet.nodes.inet.Router;
import inet.nodes.inet.StandardHost;
import inet.nodes.inet.WirelessHost;
import inet.world.radio.ChannelControl;


//
// TODO auto-generated module
//
network WirelessNetWithDHCP
{
    @display("bgb=590,407;bgl=2");
    int numWirelessHosts;
    submodules:
        channelControl: ChannelControl {
            @display("p=64,34");
            pMax = 100mW;
            numChannels = 5;
        }
        router: Router {
            @display("p=241,136");
            routingFile = "router.rt";
        }
        standardHost: StandardHost {
            @display("p=418,134");
            routingFile = "host.rt";
        }
        wirelessHost[numWirelessHosts]: WirelessHost {
            //@display("p=135,333");
            numUdpApps = 1;
            udpApp[0].typename = "DHCPClient";
        }

        wirelessRouter: WirelessHost {
            @display("p=154,207");
            routingFile = "wirelessRouter.rt";
            numUdpApps = 1;
            udpApp[0].typename = "DHCPServer";
            wlan[0].mgmtType = default("Ieee80211MgmtAP");
            wlan[0].mgmt.encapDecap = "true";
        }
    connections:
        standardHost.ethg++ <--> Eth100M <--> router.ethg++;
        wirelessRouter.ethg++ <--> Eth100M <--> router.ethg++;
}
