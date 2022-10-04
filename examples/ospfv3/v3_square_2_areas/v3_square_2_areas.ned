package inet.examples.ospfv3.v3_square_2_areas;

import inet.common.misc.ThruputMeteringChannel;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.StandardHost;
import inet.node.ospfv3.Ospfv3Router;


network v3_square_2_areas
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
                             "<interface among='Router1 Host1' address='172.16.10.x' netmask='255.255.255.0' />"+
                            "</config>");
                @display("p=147,54");
        }
        Router1: Ospfv3Router {
            parameters:
                @display("p=101,256");
            gates:
                ethg[3];
        }
        Router2: Ospfv3Router {
            parameters:
                @display("p=101,115");
            gates:
                ethg[2];
        }
        Router3: Ospfv3Router {
            parameters:
                @display("p=341,115");
            gates:
                ethg[2];
        }
        Router4: Ospfv3Router {
            parameters:
                @display("p=341,256");
            gates:
                ethg[2];
        }
        Host1: StandardHost {
            @display("p=28,256");
            gates:
                ethg[1];
        }

    connections:
        Router1.ethg[0] <--> Channel <--> Router2.ethg[0];
        Router2.ethg[1] <--> Channel <--> Router3.ethg[0];
        Router3.ethg[1] <--> Channel <--> Router4.ethg[1];
        Router4.ethg[0] <--> Channel <--> Router1.ethg[1];
        Router1.ethg[2] <--> Channel <--> Host1.ethg[0];



}
