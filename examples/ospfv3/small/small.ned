package inet.examples.ospfv3.small;

import inet.node.ospfv3.Ospfv3Router;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.networklayer.configurator.ipv6.Ipv6FlatNetworkConfigurator;
import inet.common.misc.ThruputMeteringChannel;
import inet.node.ethernet.EtherSwitch;
import inet.node.inet.StandardHost;

network small
{
    @display("bgb=844,488");
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
                            "<interface among='Router1 Router2 Router3 Router4' address='192.168.1.x' netmask='255.255.255.0' />"+
							"<interface among='Router1 Host1' address='172.16.10.x' netmask='255.255.255.0' />"+
							"<interface among='Router2 Host2' address='172.16.20.x' netmask='255.255.255.0' />"+
							"<interface among='Router3 Host3' address='172.16.30.x' netmask='255.255.255.0' />"+
							"<interface among='Router4 Host4' address='172.16.40.x' netmask='255.255.255.0' />"+
                            "</config>");
                @display("p=211,62");
        }
        Router1: Ospfv3Router {
            parameters:
                @display("p=273,390");
            gates:
                ethg[2];
        }
        Router2: Ospfv3Router {
            parameters:
                @display("p=592,359");
            gates:
                ethg[2];
        }
        etherSwitch: EtherSwitch {
            @display("p=408,245");
        }
        Router4: Ospfv3Router {
            @display("p=592,121");
            gates:
                ethg[2];
        }
        Router3: Ospfv3Router {
            @display("p=289,140");
            gates:
                ethg[2];
        }
        Host1: StandardHost {
            @display("p=67,398");
            gates:
                ethg[1];
        }
        Host2: StandardHost {
            @display("p=783,359");
            gates:
                ethg[1];
        }
        Host4: StandardHost {
            @display("p=766,117");
            gates:
                ethg[1];
        }
        Host3: StandardHost {
            @display("p=51,140");
            gates:
                ethg[1];
        }
    connections:
        Router1.ethg[0] <--> Channel <--> etherSwitch.ethg++;
        Router2.ethg[0] <--> Channel <--> etherSwitch.ethg++;
        Router3.ethg[0] <--> Channel <--> etherSwitch.ethg++;
        Router4.ethg[0] <--> Channel <--> etherSwitch.ethg++;

        Router1.ethg[1] <--> Channel <--> Host1.ethg[0];
        Router2.ethg[1] <--> Channel <--> Host2.ethg[0];
        Router3.ethg[1] <--> Channel <--> Host3.ethg[0];
        Router4.ethg[1] <--> Channel <--> Host4.ethg[0];
}
