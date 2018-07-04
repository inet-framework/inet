

package inet.examples.emulation.tap;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.emulation.ExtHost;
import inet.node.ethernet.Eth100M;
import inet.node.ethernet.EtherSwitch;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import inet.visualizer.integrated.IntegratedVisualizer;

network TapNet
{
    @display("bgb=400,400");
    submodules:
        host: StandardHost {
            @display("p=250,181");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=76,61");
        }
        visualizer: IntegratedVisualizer {
            @display("p=250,51");
        }
        tapHost: ExtHost {
            @display("p=95,181");
        }
    connections:
        tapHost.ethg++ <--> Eth100M <--> host.ethg++;
}

network TapNetSwitch
{
    @display("bgb=400,400");
    submodules:
        host: StandardHost {
            @display("p=272,173");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=76,61");
        }
        visualizer: IntegratedVisualizer {
            @display("p=250,51");
        }
        switch: EtherSwitch {
            @display("p=182,289");
        }
        tapHost: ExtHost {
            @display("p=82,173");
        }
    connections:
        host.ethg++ <--> Eth100M <--> switch.ethg++;
        tapHost.ethg++ <--> Eth100M <--> switch.ethg++;
}
