package inet.examples.bgpv4.BgpDiamondFailover;

import inet.common.misc.ThruputMeteringChannel;
import inet.common.scenario.ScenarioManager;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import inet.visualizer.canvas.integrated.IntegratedCanvasVisualizer;

network BgpDiamondFailover
{
    types:
        channel FastLink extends ThruputMeteringChannel
        {
            parameters:
                delay = 50us;
                datarate = 100Mbps;
                displayAsTooltip = true;
                thruputDisplayFormat = "#N";
        }
        channel SlowLink extends ThruputMeteringChannel
        {
            parameters:
                delay = 300us;
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
                @display("p=220,200;i=device/laptop");
        }
        A: Router {
            parameters:
                hasBgp = true;
                hasStatus = true;
                @display("p=360,200");
            gates:
                ethg[3];
        }
        B: Router {
            parameters:
                hasBgp = true;
                hasStatus = true;
                @display("p=520,120");
            gates:
                ethg[2];
        }
        D: Router {
            parameters:
                hasBgp = true;
                hasStatus = true;
                @display("p=520,280");
            gates:
                ethg[2];
        }
        C: Router {
            parameters:
                hasBgp = true;
                hasStatus = true;
                @display("p=700,200");
            gates:
                ethg[3];
        }
        hostC: StandardHost {
            parameters:
                @display("p=860,200;i=device/laptop");
        }
    connections:
        hostA.ethg++ <--> FastLink <--> A.ethg[0];
        A.ethg[1] <--> FastLink <--> B.ethg[0];
        B.ethg[1] <--> FastLink <--> C.ethg[0];
        A.ethg[2] <--> SlowLink <--> D.ethg[0];
        D.ethg[1] <--> SlowLink <--> C.ethg[1];
        C.ethg[2] <--> FastLink <--> hostC.ethg++;
}
