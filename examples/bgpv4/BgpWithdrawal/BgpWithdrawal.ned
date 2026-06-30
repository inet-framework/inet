package inet.examples.bgpv4.BgpWithdrawal;

import inet.common.misc.ThruputMeteringChannel;
import inet.common.scenario.ScenarioManager;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import inet.visualizer.canvas.integrated.IntegratedCanvasVisualizer;

//
// A linear pure-BGP chain (hostA - A - B - C - hostC) with no path redundancy.
// Each router is in its own AS. A originates 10.0.1.0/24 and C originates
// 10.0.3.0/24. When A is shut down, B loses its only path to 10.0.1.0/24 and
// must send an explicit BGP withdrawal to C over the B-C session, which stays
// up the whole time. This exercises the withdrawal-send path that the diamond
// failover example cannot reach (there an alternate path always survives).
//
network BgpWithdrawal
{
    types:
        channel Link extends ThruputMeteringChannel
        {
            parameters:
                delay = 50us;
                datarate = 100Mbps;
                displayAsTooltip = true;
                thruputDisplayFormat = "#N";
        }
    submodules:
        visualizer: IntegratedCanvasVisualizer {
            parameters:
                @display("p=100,100;is=s");
        }
        configurator: Ipv4NetworkConfigurator {
            parameters:
                @display("p=100,200;is=s");
                config = xmldoc("IPv4Config.xml");
                addStaticRoutes = false;
                addDefaultRoutes = false;
                addSubnetRoutes = false;
        }
        scenarioManager: ScenarioManager {
            parameters:
                @display("p=100,300;is=s");
        }
        hostA: StandardHost {
            parameters:
                @display("p=160,200;i=device/laptop");
        }
        A: Router {
            parameters:
                hasBgp = true;
                hasStatus = true;
                @display("p=300,200");
            gates:
                ethg[2];
        }
        B: Router {
            parameters:
                hasBgp = true;
                hasStatus = true;
                @display("p=460,200");
            gates:
                ethg[2];
        }
        C: Router {
            parameters:
                hasBgp = true;
                hasStatus = true;
                @display("p=620,200");
            gates:
                ethg[2];
        }
        hostC: StandardHost {
            parameters:
                @display("p=760,200;i=device/laptop");
        }
    connections:
        hostA.ethg++ <--> Link <--> A.ethg[0];
        A.ethg[1] <--> Link <--> B.ethg[0];
        B.ethg[1] <--> Link <--> C.ethg[0];
        C.ethg[1] <--> Link <--> hostC.ethg++;
}
