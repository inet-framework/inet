
package inet.examples.rip.dynamictest;

import inet.common.misc.ThruputMeteringChannel;
import inet.common.scenario.ScenarioManager;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.networklayer.ipv4.RoutingTableRecorder;
import inet.node.inet.StandardHost;
import inet.node.rip.RipRouter;


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
            @display("p=100,200;is=s");
        }
        H1: StandardHost {
            parameters:
                @display("p=250,200");
            gates:
                ethg[1];
        }
        R1: RipRouter {
            parameters:
                @display("p=400,200");
            gates:
                ethg[3];
        }
        RA: RipRouter {
            parameters:
                @display("p=550,100");
            gates:
                ethg[2];
        }
        RB: RipRouter {
            parameters:
                @display("p=550,300");
            gates:
                ethg[2];
        }
        R2: RipRouter {
            parameters:
                @display("p=700,200");
            gates:
                ethg[3];
        }
        H2: StandardHost {
            parameters:
                @display("p=850,200");
            gates:
                ethg[1];
        }
        configurator: Ipv4NetworkConfigurator {
            parameters:
                @display("p=100,100;is=s");
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
        scenarioManager: ScenarioManager {
            @display("p=100,300;is=s");
        }
    connections:
        H1.ethg[0] <--> C <--> R1.ethg[0];
        R1.ethg[1] <--> C <--> RA.ethg[0];
        R1.ethg[2] <--> C <--> RB.ethg[0];
        RA.ethg[1] <--> C <--> R2.ethg[1];
        RB.ethg[1] <--> C <--> R2.ethg[2];
        R2.ethg[0] <--> C <--> H2.ethg[0];
}

