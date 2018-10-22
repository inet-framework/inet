

package inet.showcases.emulation.videostreaming;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ethernet.Eth100G;
import inet.node.ethernet.Eth100M;
import inet.node.ethernet.EtherSwitch;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import inet.visualizer.integrated.IntegratedVisualizer;

network VideoStreamingShowcase
{
    @display("bgb=400,400");
    submodules:
        incomingHost: StandardHost {
            @display("p=64,129;i2=misc/rightdown;i=misc/laptop2");
        }
        outgoingHost: StandardHost {
            @display("p=328,129;i2=misc/rightup;i=misc/laptop2");
        }
        switch: EtherSwitch {
            @display("p=198,239");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=51,36");
        }
        visualizer: IntegratedVisualizer {
            @display("p=145,39");
        }
    connections:
        incomingHost.ethg++ <--> Eth100M <--> switch.ethg++;
        switch.ethg++ <--> Eth100M <--> outgoingHost.ethg++;
}

network VideoStreamingShowcaseOneHost
{
    @display("bgb=400,400");
    submodules:
        host: StandardHost {
            @display("p=202,197;i2=misc/rightdown;i=misc/laptop2");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=51,36");
        }
        visualizer: IntegratedVisualizer {
            @display("p=145,39");
        }
    connections:
}

network VideoStreamingShowcaseNoSwitch
{
    @display("bgb=400,400");
    submodules:
        incomingHost: StandardHost {
            @display("p=64,129;i2=misc/rightdown;i=misc/laptop2");
        }
        outgoingHost: StandardHost {
            @display("p=328,129;i2=misc/rightup;i=misc/laptop2");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=51,36");
        }
        visualizer: IntegratedVisualizer {
            @display("p=145,39");
        }
    connections:
        incomingHost.ethg++ <--> Eth100M <--> outgoingHost.ethg++;
}

network SwitchWithExtInterfaceTest
{
    @display("bgb=400,400");
    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=30,36");
        }
        visualizer: IntegratedVisualizer {
            @display("p=178,29");
        }
        switch: EtherSwitch {
            @display("p=117,183");
        }
}

network SwitchWithExtInterfaceTest2
{
    @display("bgb=400,400");
    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=30,36");
        }
        visualizer: IntegratedVisualizer {
            @display("p=178,29");
        }
        switch: EtherSwitch {
            @display("p=117,183");
        }
        etherSwitch: EtherSwitch {
            @display("p=285,188");
        }
    connections:
        switch.ethg++ <--> Eth100M <--> etherSwitch.ethg++;
}

network VideoStreamingWithSwitches
{
    @display("bgb=400,400");
    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=70,71");
        }
        visualizer: IntegratedVisualizer {
            @display("p=194,71");
        }
        switch1: EtherSwitch {
            @display("p=62,216");
        }
        switch2: EtherSwitch {
            @display("p=317,216");
        }
        router: Router {
            @display("p=184,217");
        }
    connections:
        switch1.ethg++ <--> Eth100M <--> router.ethg++;
        router.ethg++ <--> Eth100M <--> switch2.ethg++;
}

network VideoStreamingWithRouters
{
    @display("bgb=400,400");
    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=70,59");
        }
        visualizer: IntegratedVisualizer {
            @display("p=186,59");
        }
        router1: Router {
            @display("p=73,175;t=ext-->,l");
        }
        router2: Router {
            @display("p=302,175;t=-->ext,r");
        }
        switch: EtherSwitch {
            @display("p=186,174");
        }
    connections:
        router1.ethg++ <--> Eth100M <--> switch.ethg++;
        switch.ethg++ <--> Eth100M <--> router2.ethg++;
}

network VideoStreamingPing
{
    @display("bgb=400,400");
    submodules:
        host: StandardHost {
            @display("p=184,181");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=76,61");
        }
        visualizer: IntegratedVisualizer {
            @display("p=250,51");
        }
}

network VideoStreamingPingSwitch
{
    @display("bgb=400,400");
    submodules:
        host: StandardHost {
            @display("p=184,181");
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
    connections:
        host.ethg++ <--> Eth100M <--> switch.ethg++;
}

network VideoStreamingExtHost
{
    @display("bgb=400,400");
    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=70,71");
        }
        visualizer: IntegratedVisualizer {
            @display("p=194,71");
        }
        switch1: EtherSwitch {
            @display("p=119,216");
        }
        switch2: EtherSwitch {
            @display("p=273,216");
        }
        router: Router {
            @display("p=194,217");
        }
        extHost1: StandardHost {
            @display("p=45,217;i=,white,100");
        }
        extHost2: StandardHost {
            @display("p=357,217;i=,white,100");
        }
    connections:
        switch1.ethg++ <--> Eth100G <--> router.ethg++;
        router.ethg++ <--> Eth100G <--> switch2.ethg++;
        extHost1.ethg++ <--> Eth100G <--> switch1.ethg++;
        switch2.ethg++ <--> Eth100G <--> extHost2.ethg++;
}
