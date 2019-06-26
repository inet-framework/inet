

package inet.showcases.visualizer.transportconnection;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ethernet.Eth100M;
import inet.node.ethernet.EtherSwitch;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import inet.node.inet.WirelessHost;
import inet.visualizer.integrated.IntegratedVisualizer;


network TransportConnectionVisualizationEnablingShowcase
{
    @display("bgb=596,373");
    submodules:
        host1: StandardHost {
            @display("p=78,133");
        }
        host2: StandardHost {
            @display("p=487,133");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=109,290");
        }
        visualizer: IntegratedVisualizer {
            @display("p=207,290");
        }
    connections:
        host1.ethg++ <--> Eth100M <--> host2.ethg++;
}

network TransportConnectionVisualizationMultipleConnectionsShowcase
{
    @display("bgb=596,421");
    submodules:
        host1: StandardHost {
            @display("p=59,103");
        }
        host2: StandardHost {
            @display("p=59,329");
        }
        switch: EtherSwitch {
            @display("p=199,213");
        }
        router: Router {
            @display("p=299,214");
        }
        server: StandardHost {
            @display("p=410,214;i=device/server");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=531,53");
        }
        visualizer: IntegratedVisualizer {
            @display("p=531,119");
        }
    connections:
        host1.ethg++ <--> Eth100M <--> switch.ethg++;
        host2.ethg++ <--> Eth100M <--> switch.ethg++;
        switch.ethg++ <--> Eth100M <--> router.ethg++;
        router.ethg++ <--> Eth100M <--> server.ethg++;
}

network Complexer
{
    @display("bgb=596,421");
    submodules:
        host1: StandardHost {
            @display("p=59,103");
        }
        host2: StandardHost {
            @display("p=59,329");
        }
        switch: EtherSwitch {
            @display("p=199,213");
        }
        router: Router {
            @display("p=299,214");
        }
        server: StandardHost {
            @display("p=410,214;i=device/server");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=531,53");
        }
        visualizer: IntegratedVisualizer {
            @display("p=531,119");
        }
        terminalHost: StandardHost {
            @display("p=362,88;i=device/terminal");
        }
    connections:
        host1.ethg++ <--> Eth100M <--> switch.ethg++;
        host2.ethg++ <--> Eth100M <--> switch.ethg++;
        switch.ethg++ <--> Eth100M <--> router.ethg++;
        router.ethg++ <--> Eth100M <--> server.ethg++;
        terminalHost.ethg++ <--> Eth100M <--> server.ethg++;
}
