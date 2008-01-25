//
// Copyright (C) 2004 Emin Ilker Cetinbas (niw3@yahoo.com)
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



package org.omnetpp.inet.examples.Ethernet.ARPTest2;

import org.omnetpp.inet.Network.AutoRouting.FlatNetworkConfigurator;
import org.omnetpp.inet.NetworkInterfaces.Ethernet.EtherBus;
import org.omnetpp.inet.Nodes.INET.Router;
import org.omnetpp.inet.Nodes.INET.StandardHost;


network ARPTest
{
    submodules:
        server: StandardHost {
            parameters:
                @display("p=480,198;i=device/server_l");
        }
        router8: Router {
            parameters:
                @display("p=478,100");
        }
        router7: Router {
            parameters:
                @display("p=329,100");
        }
        router6: Router {
            parameters:
                @display("p=254,100");
        }
        bus2: EtherBus {
            parameters:
                positions = "";
                propagationSpeed = 2e8;
                writeScalars = false;
                @display("p=260,40;b=260,10");
        }
        router5: Router {
            parameters:
                @display("p=187,98");
        }
        bus1: EtherBus {
            parameters:
                positions = "";
                propagationSpeed = 2e8;
                writeScalars = false;
                @display("p=280,170;b=260,10");
        }
        router4: Router {
            parameters:
                @display("p=386,240");
        }
        router3: Router {
            parameters:
                @display("p=320,240");
        }
        router2: Router {
            parameters:
                @display("p=247,240");
        }
        router1: Router {
            parameters:
                @display("p=182,283");
        }
        client: StandardHost {
            parameters:
                @display("p=54,274;i=device/laptop_l");
        }
        configurator: FlatNetworkConfigurator {
            parameters:
                moduleTypes = "Router StandardHost EtherBus";
                nonIPModuleTypes = "EtherBus";
                networkAddress = "10.0.0.0";
                netmask = "255.255.0.0";
                @display("p=46,111");
        }
    connections:
        server.ethg++ <--> router8.ethg++;
        bus2.ethg++ <--> router5.ethg++;
        bus2.ethg++ <--> router6.ethg++;
        bus2.ethg++ <--> router7.ethg++;
        router8.ethg++ <--> router7.ethg++;
        bus1.ethg++ <--> router1.ethg++;
        bus1.ethg++ <--> router5.ethg++;
        bus1.ethg++ <--> router2.ethg++;
        bus1.ethg++ <--> router3.ethg++;
        bus1.ethg++ <--> router4.ethg++;
        client.ethg++ <--> router1.ethg++;
}

