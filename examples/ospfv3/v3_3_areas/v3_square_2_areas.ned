package inet.examples.ospfv3.v3_3_areas;

import inet.common.misc.ThruputMeteringChannel;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ospfv3.Ospfv3Router;


network v3_3_areas
{
    types:
        channel Channel extends ThruputMeteringChannel
        {
            delay = 0.1us;
            datarate = 100Mbps;
            thruputDisplayFormat = "#N";
        }
    submodules:
        configurator: Ipv4NetworkConfigurator {
            parameters:
                config = xml("<config>"+
                            "</config>"
                            );
                addStaticRoutes = false;
                addDefaultRoutes = false;
                addSubnetRoutes = false;
                @display("p=147,54");
        }
        Router1: Ospfv3Router {
            parameters:
                @display("p=63,163");
            gates:
                ethg[1];
        }
        Router2: Ospfv3Router {
            parameters:
                @display("p=186,163");
            gates:
                ethg[3];
        }
        Router3: Ospfv3Router {
            parameters:
                @display("p=302,89");
            gates:
                ethg[1];
        }
        Router4: Ospfv3Router {
            parameters:
                @display("p=320,207");
            gates:
                ethg[1];
        }

    connections:
        Router1.ethg[0] <--> Channel <--> Router2.ethg[0];
        Router2.ethg[1] <--> Channel <--> Router3.ethg[0];
        Router2.ethg[2] <--> Channel <--> Router4.ethg[0];

}
