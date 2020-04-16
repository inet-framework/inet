//
// Copyright (C) 2014 Irene Ruengeler
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




import inet.node.packetdrill.PacketDrillHost;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;


network PacketDrillSctp
{
    submodules:
        pdhost: PacketDrillHost {
            parameters:
              //  ipv4.configurator.networkConfiguratorModule = "";
                @display("p=544,194;i=device/server_l");
        }
        configurator: Ipv4NetworkConfigurator {
            parameters:
                addDefaultRoutes = false;
                addStaticRoutes = false;
                addSubnetRoutes = false;
                config = xml(
                    "<config>"+"\n"+
                    "  <interface hosts='pdhost' names='tun0' address='192.168.0.1' netmask='255.255.255.0' mtu='1500' metric='1'/>"+"\n"+
                    "  <route hosts='pdhost' destination='192.168.0.2' netmask='255.255.255.255' interface='tun0'/>"+"\n"+
                    "</config>"
                );
                @display("p=185,50");
        }
    connections:
}

