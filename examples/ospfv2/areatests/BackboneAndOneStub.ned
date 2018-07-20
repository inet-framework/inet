

package inet.examples.ospfv2.areatests;

import inet.common.misc.ThruputMeteringChannel;
import inet.common.scenario.ScenarioManager;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;


network OSPF_BackboneAndOneStubTest
{
    types:
        channel C extends ThruputMeteringChannel
        {
            delay = 0.1us;
            datarate = 100Mbps;
            thruputDisplayFormat = "#N";
        }
    submodules:
        Backbone: TestArea {
            parameters:
                extGates = 1;
                @display("p=300,150;b=100,100");
        }
        Stub1: TestArea {
            parameters:
                extGates = 1;
                @display("p=500,150;b=100,100");
        }
        configurator: Ipv4NetworkConfigurator {
            parameters:
                config = xml("<config>"+
                            "<interface hosts='**.B[*]' names='eth0' address='192.168.3.x' netmask='255.255.255.x' />"+
                            "<interface hosts='Backbone.*' address='192.168.1.x' netmask='255.255.255.x' />"+
                            "<interface hosts='Stub1.*' address='192.168.2.x' netmask='255.255.255.x' />"+
                            "<multicast-group hosts='**.R[*]' address='224.0.0.5 224.0.0.6' />"+
                            "<route hosts='**.H[*]' destination='*' netmask='0.0.0.0' interface='eth0' />"+
                            "<route hosts='**.R[*] **.B[*]' destination='224.0.0.0' netmask='240.0.0.0' interface='eth0' />"+
                            "<route hosts='**.R[*] **.B[*]' destination='224.0.0.0' netmask='240.0.0.0' interface='eth1' />"+
                            "<route hosts='**.R[*] **.B[*]' destination='224.0.0.0' netmask='240.0.0.0' interface='eth2' />"+
                            "</config>");
                addStaticRoutes = false;
                addDefaultRoutes = false;
                @display("p=100,100;is=s");
        }
        scenarioManager: ScenarioManager {
            @display("p=100,200;is=s");
        }
    connections:
        Backbone.ethg[0] <--> C <--> Stub1.ethg[0];
}
