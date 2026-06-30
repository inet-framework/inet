package inet.examples.bgpv4.BgpWithdrawal6;

import inet.common.misc.ThruputMeteringChannel;
import inet.common.scenario.ScenarioManager;
import inet.networklayer.configurator.ipv6.Ipv6NetworkConfigurator;
import inet.node.inet.Router;
import inet.node.ipv6.StandardHost6;
import inet.visualizer.canvas.integrated.IntegratedCanvasVisualizer;

//
// IPv6 (MP-BGP) counterpart of ~BgpWithdrawal: a linear chain
// (hostA - A - B - C - hostC) with no path redundancy. When A is shut down,
// B loses its only path to 2001:db8:1::/64 and sends an explicit withdrawal to
// C in an MP_UNREACH_NLRI attribute (RFC 4760) over the B-C session, which
// stays up. This exercises the IPv6 withdrawal-send path.
//
network BgpWithdrawal6
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
        configurator: Ipv6NetworkConfigurator {
            parameters:
                @display("p=100,200;is=s");
                config = xmldoc("Ipv6Config.xml");
                addRemoteRoutes = false;
        }
        scenarioManager: ScenarioManager {
            parameters:
                @display("p=100,300;is=s");
        }
        hostA: StandardHost6 {
            parameters:
                @display("p=160,200;i=device/laptop");
        }
        A: Router {
            parameters:
                hasBgp = true;
                hasStatus = true;
                hasIpv4 = false;
                hasIpv6 = true;
                @display("p=300,200");
            gates:
                ethg[2];
        }
        B: Router {
            parameters:
                hasBgp = true;
                hasStatus = true;
                hasIpv4 = false;
                hasIpv6 = true;
                @display("p=460,200");
            gates:
                ethg[2];
        }
        C: Router {
            parameters:
                hasBgp = true;
                hasStatus = true;
                hasIpv4 = false;
                hasIpv6 = true;
                @display("p=620,200");
            gates:
                ethg[2];
        }
        hostC: StandardHost6 {
            parameters:
                @display("p=760,200;i=device/laptop");
        }
    connections:
        hostA.ethg++ <--> Link <--> A.ethg[0];
        A.ethg[1] <--> Link <--> B.ethg[0];
        B.ethg[1] <--> Link <--> C.ethg[0];
        C.ethg[1] <--> Link <--> hostC.ethg++;
}
