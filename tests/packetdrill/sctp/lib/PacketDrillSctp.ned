//
// Copyright (C) 2014 Irene Ruengeler
//
// SPDX-License-Identifier: GPL-2.0-or-later
//
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

