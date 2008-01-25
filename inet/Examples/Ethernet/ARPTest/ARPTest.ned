//
// Copyright (C) 2006 Andras Varga
//
// This library is free software, you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation;
// either version 2 of the License, or any later version.
// The library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//


package org.omnetpp.inet.examples.Ethernet.ARPTest;

import ned.cBasicChannel;
import org.omnetpp.inet.Network.AutoRouting.FlatNetworkConfigurator;
import org.omnetpp.inet.NetworkInterfaces.EtherSwitch.EtherSwitch;
import org.omnetpp.inet.Nodes.INET.Router;
import org.omnetpp.inet.Nodes.INET.StandardHost;


network ARPTest
{
    types:
        channel fiberline extends cBasicChannel
        {
            delay = 1us;
            datarate = 512Mbps;
        }
    submodules:
        client: StandardHost {
            parameters:
                @display("p=71,64;i=device/laptop_l");
        }
        host1: StandardHost {
            parameters:
                @display("p=65,131;i=device/pc");
        }
        host2: StandardHost {
            parameters:
                @display("p=60,191;i=device/pc");
        }
        switch: EtherSwitch {
            parameters:
                @display("p=202,156");
        }
        net: Router {
            parameters:
                @display("p=394,166");
        }
        router: Router {
            parameters:
                @display("p=311,74");
        }
        server: StandardHost {
            parameters:
                @display("p=512,58;i=device/server_l");
        }
        configurator: FlatNetworkConfigurator {
            parameters:
                moduleTypes = "Router StandardHost EtherSwitch";
                nonIPModuleTypes = "EtherSwitch";
                networkAddress = "10.0.0.0";
                netmask = "255.255.0.0";
                @display("p=495,160");
        }
    connections:
        client.ethg++ <--> switch.ethg++;
        switch.ethg++ <--> host1.ethg++;
        switch.ethg++ <--> host2.ethg++;
        router.ethg++ <--> switch.ethg++;
        router.pppg++ <--> fiberline <--> net.pppg++;
        server.pppg++ <--> fiberline <--> net.pppg++;
}

