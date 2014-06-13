
package inet.examples.rip.dynamictest;

import inet.common.lifecycle.LifecycleController;
import inet.common.misc.ThruputMeteringChannel;
import inet.common.scenario.ScenarioManager;
import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;
import inet.networklayer.ipv4.RoutingTableRecorder;
import inet.node.inet.StandardHost;
import inet.node.rip.RIPRouter;


network DynamicTest
{
    parameters:
        @display("p=10,10;b=712,152");
    types:
        channel C extends ThruputMeteringChannel
        {
            delay = 0.1us;
            datarate = 100Mbps;
            thruputDisplayFormat = "#N";
        }
    submodules:
        rtr: RoutingTableRecorder {
            @display("p=184,42");
        }
        H1: StandardHost {
            parameters:
                @display("p=50,123");
            gates:
                ethg[1];
        }
        R1: RIPRouter {
            parameters:
                @display("p=192,123");
            gates:
                ethg[3];
        }
        RA: RIPRouter {
            parameters:
                @display("p=351,42");
            gates:
                ethg[2];
        }
        RB: RIPRouter {
            parameters:
                @display("p=351,194");
            gates:
                ethg[2];
        }
        R2: RIPRouter {
            parameters:
                @display("p=486,123");
            gates:
                ethg[3];
        }
        H2: StandardHost {
            parameters:
                @display("p=666,123");
            gates:
                ethg[1];
        }
        configurator: IPv4NetworkConfigurator {
            parameters:
                @display("p=94,42");
                config = xml("<config>"
                        + "<interface among='H1 R1' address='192.168.1.x' netmask='255.255.255.x' />"
                        + "<interface among='H2 R2' address='192.168.2.x' netmask='255.255.255.x' />"
                        + "<interface among='R*' address='10.0.0.x' netmask='255.255.255.x' />"
                        + "<route hosts='H1' destination='*' gateway='R1'/>"
                        + "<route hosts='H2' destination='*' gateway='R2'/>"
                        + "<route hosts='R*' destination='224.0.0.0' netmask='240.0.0.0' interface='eth0'/>"
                        + "<route hosts='R*' destination='224.0.0.0' netmask='240.0.0.0' interface='eth1'/>"
                        + "<route hosts='R1 R2' destination='224.0.0.0' netmask='240.0.0.0' interface='eth2'/>"
                        + "</config>");
        }
        lifecycleController: LifecycleController {
            @display("p=532,42");
        }
        scenarioManager: ScenarioManager {
            @display("p=658,42");
        }
    connections:
        H1.ethg[0] <--> C <--> R1.ethg[0];
        R1.ethg[1] <--> C <--> RA.ethg[0];
        R1.ethg[2] <--> C <--> RB.ethg[0];
        RA.ethg[1] <--> C <--> R2.ethg[1];
        RB.ethg[1] <--> C <--> R2.ethg[2];
        R2.ethg[0] <--> C <--> H2.ethg[0];
}

