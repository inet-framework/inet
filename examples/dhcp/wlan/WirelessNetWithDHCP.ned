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
import inet.networklayer.autorouting.ipv4.IPv4NetworkConfigurator;
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
        }
        standardHost: StandardHost {
            @display("p=418,134");
        }
        wirelessHost[numWirelessHosts]: WirelessHost {
            numUdpApps = 1;
            udpApp[0].typename = "DHCPClient";
        }
        wirelessRouter: WirelessHost {
            @display("p=154,207");
            numUdpApps = 1;
            udpApp[0].typename = "DHCPServer";
            IPForward = true;
            networkLayer.proxyARP = true;
            wlan[0].mgmtType = default("Ieee80211MgmtAP");
            wlan[0].mgmt.encapDecap = "true";
        }
        configurator: IPv4NetworkConfigurator {
            @display("p=71,374");
            config = xml(
                "<config>\n" +
                "  <interface hosts='standardHost' towards='router' address='10.0.1.1'/>\n" +
                "  <interface hosts='router' towards='standardHost' address='10.0.1.2'/>\n" +
                "  <interface hosts='router' towards='wirelessRouter' address='10.0.0.1'/>\n" +
                "  <interface hosts='wirelessRouter' towards='router' address='10.0.0.2' netmask='255.255.255.252'/>\n" +
                "  <interface hosts='wirelessRouter' names='wlan0' address='192.168.0.1' netmask='255.255.255.0'/>\n" +
                "  <route hosts='standardHost' destination='*' netmask='*' gateway='router'/>\n" +
                "  <route hosts='router' destination='192.168.0.0' netmask='255.255.255.0' gateway='wirelessRouter'/>\n" +
                "  <route hosts='wirelessRouter' destination='192.168.0.0' netmask='255.255.255.0' interface='wlan0'/>\n" +
                "  <route hosts='wirelessRouter' destination='10.0.0.0' netmask='255.255.0.0' gateway='router'/>\n" +
                "</config>\n");
            addStaticRoutes = false;
            addDefaultRoutes = false;
            addSubnetRoutes = false;
        }
    connections:
        standardHost.ethg++ <--> Eth100M <--> router.ethg++;
        wirelessRouter.ethg++ <--> Eth100M <--> router.ethg++;
}
