package inet.examples.ospfv3.v3_4_routers_2_areas;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ospfv3.Ospfv3Router;
import ned.TransmissionChannel;


network v3_4_routers_2_areas
{
    parameters:
        *.eth[*].bitrate = 100 Mbps;
    types:
        channel Channel extends TransmissionChannel
        {
            delay = 0.1us;
        }
    submodules:
        configurator: Ipv4NetworkConfigurator {
            parameters:
                config = xml("<config>"+
                            "<interface among='Router1 Router2' address='10.10.10.x' netmask='255.255.255.0' />"+
                            "<interface among='Router2 Router3' address='10.10.20.x' netmask='255.255.255.0' />"+
                            "<interface among='Router3 Router4' address='10.10.30.x' netmask='255.255.255.0' />"+
                            "<interface among='Router3 Router5' address='10.10.40.x' netmask='255.255.255.0' />"+
                            "</config>");
                @display("p=147,54");
        }
        Router1: Ospfv3Router {
            parameters:
                @display("p=56,230");
            gates:
                ethg[1];
        }
        Router2: Ospfv3Router {
            parameters:
                @display("p=213,153");
            gates:
                ethg[2];
        }
        Router3: Ospfv3Router {
            parameters:
                @display("p=376,153");
            gates:
                ethg[3];
        }
        Router4: Ospfv3Router {
            parameters:
                @display("p=519,50");
            gates:
                ethg[1];
        }
        Router5: Ospfv3Router {
            parameters:
                @display("p=508,212");
            gates:
                ethg[1];
        }
    connections:
        Router1.ethg[0] <--> Channel <--> Router2.ethg[0];
        Router2.ethg[1] <--> Channel <--> Router3.ethg[0];
        Router3.ethg[1] <--> Channel <--> Router4.ethg[0];
        Router3.ethg[2] <--> Channel <--> Router5.ethg[0];
}
