

package inet.samples.visualizer.transportconnection;

import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;
import inet.node.ethernet.Eth100M;
import inet.node.ethernet.EtherSwitch;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import inet.visualizer.integrated.IntegratedVisualizer;


network Simple
{
    @display("bgb=596,479");
    submodules:
        host1: StandardHost {
            @display("p=78,133");
        }
        host2: StandardHost {
            @display("p=487,133");
        }
        router: Router {
            @display("p=279,307");
        }
        etherSwitch: EtherSwitch {
            @display("p=174,214");
        }
        configurator: IPv4NetworkConfigurator {
            @display("p=108,408");
        }
        visualizer: IntegratedVisualizer {
            @display("p=318,407");
        }
    connections:
        host1.ethg++ <--> Eth100M <--> etherSwitch.ethg++;
        etherSwitch.ethg++ <--> Eth100M <--> router.ethg++;
        router.ethg++ <--> Eth100M <--> host2.ethg++;
}
