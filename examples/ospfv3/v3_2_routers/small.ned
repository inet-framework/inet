package inet.examples.ospfv3.v3_2_routers;

import inet.node.ospfv3.Ospfv3Router;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.networklayer.configurator.ipv6.Ipv6FlatNetworkConfigurator;
import inet.common.misc.ThruputMeteringChannel;
import inet.node.ethernet.EtherSwitch;

import inet.common.scenario.ScenarioManager;


network v3_2_routers
{
    types:
        channel Channel extends ThruputMeteringChannel
        {
            delay = 0.1us;
            datarate = 100Mbps;
            thruputDisplayFormat = "#N";
        }
    submodules:
        Router1: Ospfv3Router {
            parameters:
                @display("p=90,154");
            gates:
                ethg[1];
        }
        Router2: Ospfv3Router {
            parameters:
                @display("p=244,97");
            gates:
                ethg[1];
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=50,20;is=s");
         	config = xmldoc("IPv4Config.xml");
            addStaticRoutes = false;
            addDefaultRoutes = false;
            addSubnetRoutes = false;
        }
        scenarioManager: ScenarioManager {
            @display("p=22.6325,96.7025");
        }

    connections:
        Router1.ethg[0] <--> Channel <--> Router2.ethg[0];
}
