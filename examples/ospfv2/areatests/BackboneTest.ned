

package inet.examples.ospfv2.areatests;

import inet.common.scenario.ScenarioManager;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;


network OSPF_BackboneTest
{
    submodules:
        Backbone: TestArea {
            parameters:
                numHosts = 10;
                @display("p=300,150;b=100,100");
        }
        configurator: Ipv4NetworkConfigurator {
            parameters:
                //                            "<route hosts='Backbone.R[*]' destination='224.0.0.0' netmask='240.0.0.0' interface='eth0' />"+
                config = xml("<config>"+
                            "<interface hosts='Backbone.*' address='192.168.1.x' netmask='255.255.255.x' />"+
                            "<multicast-group hosts='Backbone.R[*]' address='224.0.0.5 224.0.0.6' />"+
                            "<route hosts='Backbone.H[*]' destination='*' netmask='0.0.0.0' interface='eth0' />"+
//                            "<route hosts='Backbone.R[*]' destination='224.0.0.0' netmask='240.0.0.0' interface='eth0' />"+
                            "<route hosts='Backbone.R[*]' destination='224.0.0.0' netmask='240.0.0.0' interface='eth1' />"+
                            "<route hosts='Backbone.R[*]' destination='224.0.0.0' netmask='240.0.0.0' interface='eth2' />"+
                            "</config>");
                addStaticRoutes = false;
                addDefaultRoutes = false;
                @display("p=100,100;is=s");
        }
        scenarioManager: ScenarioManager {
            @display("p=100,200;is=s");
        }
}

