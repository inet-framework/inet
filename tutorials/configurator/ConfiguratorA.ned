//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.tutorials.configurator;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ethernet.Eth100M;
import inet.node.ethernet.Eth10M;
import inet.node.ethernet.EthernetSwitch;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import inet.visualizer.canvas.integrated.IntegratedCanvasVisualizer;


network ConfiguratorA
{
    @display("bgb=1610.5125,874.96497");
    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=100,100");
        }
        host0: StandardHost {
            @display("p=268.32748,417.6225");
        }
        host1: StandardHost {
            @display("p=227.9775,542.7075");
        }
        host2: StandardHost {
            @display("p=268.32748,736.3875");
        }
        host3: StandardHost {
            @display("p=667.7925,98.8575");
        }
        router0: Router {
            @display("p=667.7925,574.9875");
        }
        router2: Router {
            @display("p=972.435,574.9875");
        }
        router1: Router {
            @display("p=821.1225,369.20248");
        }
        host4: StandardHost {
            @display("p=843.315,70.612495");
        }
        host5: StandardHost {
            @display("p=972.435,98.8575");
        }
        host6: StandardHost {
            @display("p=1430.4075,417.6225");
        }
        host7: StandardHost {
            @display("p=1472.775,542.7075");
        }
        host8: StandardHost {
            @display("p=1430.4075,736.3875");
        }
        switch0: EthernetSwitch {
            @display("p=518.4975,574.9875");
        }
        switch2: EthernetSwitch {
            @display("p=1162.08,574.9875");
        }
        switch1: EthernetSwitch {
            @display("p=821.1225,225.95999");
        }
        visualizer: IntegratedCanvasVisualizer {
            @display("p=98.8575,209.81999");
        }
    connections:
        host2.ethg++ <--> Eth100M <--> switch0.ethg++;
        host1.ethg++ <--> Eth100M <--> switch0.ethg++;
        host0.ethg++ <--> Eth100M <--> switch0.ethg++;
        switch0.ethg++ <--> Eth100M <--> router0.ethg++;
        router0.ethg++ <--> Eth100M <--> router1.ethg++;
        router1.ethg++ <--> Eth100M <--> switch1.ethg++;
        switch1.ethg++ <--> Eth100M <--> host3.ethg++;
        switch1.ethg++ <--> Eth100M <--> host4.ethg++;
        switch1.ethg++ <--> Eth100M <--> host5.ethg++;
        router1.ethg++ <--> Eth100M <--> router2.ethg++;
        router2.ethg++ <--> Eth100M <--> switch2.ethg++;
        switch2.ethg++ <--> Eth100M <--> host6.ethg++;
        switch2.ethg++ <--> Eth100M <--> host7.ethg++;
        switch2.ethg++ <--> Eth100M <--> host8.ethg++;
        router0.ethg++ <--> Eth10M <--> router2.ethg++;
}

