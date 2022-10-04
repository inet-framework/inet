package inet.examples.ospfv3.v3_mesh;

import inet.common.misc.ThruputMeteringChannel;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ethernet.EthernetSwitch;
import inet.node.ospfv3.Ospfv3Router;


network v3_mesh
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
//                            "<interface among='Router1 Router2 Router3 Router4' address='192.168.1.x' netmask='255.255.255.0' />"+
                            "</config>");
                @display("p=301,38");
                addStaticRoutes = false;
                addDefaultRoutes = false;
                addSubnetRoutes = false;
        }
        Router1: Ospfv3Router {
            parameters:
                @display("p=38,285");
            gates:
                ethg[2];
        }
        Router2: Ospfv3Router {
            parameters:
                @display("p=388,285");
            gates:
                ethg[3];
        }
        etherSwitch: EthernetSwitch {
            @display("p=189,284");
        }
        Router3: Ospfv3Router {
            @display("p=189,396");
            gates:
                ethg[2];
        }
        Router4: Ospfv3Router {
            @display("p=48,178");
            gates:
                ethg[3];
        }
        Router5: Ospfv3Router {
            @display("p=160,112");
            gates:
                ethg[2];
        }
        Router6: Ospfv3Router {
            @display("p=340,168");
            gates:
                ethg[3];
        }

    connections:
        Router1.ethg[0] <--> Channel <--> etherSwitch.ethg++;
        Router2.ethg[0] <--> Channel <--> etherSwitch.ethg++;
        Router3.ethg[0] <--> Channel <--> etherSwitch.ethg++;
        Router2.ethg[1] <--> Channel <--> Router3.ethg[1];
        Router2.ethg[2] <--> Channel <--> Router6.ethg[2];
        Router4.ethg[0] <--> Channel <--> Router1.ethg[1];
        Router4.ethg[1] <--> Channel <--> Router5.ethg[0];
        Router4.ethg[2] <--> Channel <--> Router6.ethg[0];
        Router5.ethg[1] <--> Channel <--> Router6.ethg[1];
}
