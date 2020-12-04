package inet.tutorials.fingerprint;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ethernet.Eth100M;
import inet.node.ethernet.Eth10G;
import inet.node.ethernet.EthernetSwitch;
import inet.node.inet.AdhocHost;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import inet.physicallayer.wireless.common.contract.packetlevel.IRadioMedium;
import inet.visualizer.canvas.integrated.IntegratedMultiCanvasVisualizer;
import inet.visualizer.common.IntegratedVisualizer;

network RegressionTestingTutorialWireless
{
    @display("bgb=67.5,40");
    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=5,5");
        }
        radioMedium: <default("Ieee80211ScalarRadioMedium")> like IRadioMedium {
            @display("p=5,10");
        }
        visualizer: IntegratedVisualizer {
            @display("p=5,15");
        }
        host1: AdhocHost {
            @display("p=28.8,11;i=misc/smartphone");
        }
        host2: AdhocHost {
            @display("p=48,25.8;i=misc/smartphone");
        }
}

network RegressionTestingTutorialWired
{
    @display("bgb=846,526");
    submodules:
        router1: Router {
            @display("p=319.8125,375.0625");
        }
        router3: Router {
            @display("p=427.125,241.1875");
        }
        router2: Router {
            @display("p=530.1875,375.0625");
        }
        switch2: EthernetSwitch {
            @display("p=643.875,374");
        }
        switch3: EthernetSwitch {
            @display("p=427.125,167.875");
        }
        switch1: EthernetSwitch {
            @display("p=210.375,374");
        }
        host4: StandardHost {
            @display("p=745.875,308.125");
        }
        host5: StandardHost {
            @display("p=745.875,434.5625");
        }
        host1: StandardHost {
            @display("p=365.5,78.625");
        }
        host2: StandardHost {
            @display("p=522.75,78.625");
        }
        host3: StandardHost {
            @display("p=574.8125,168.9375");
        }
        server: StandardHost {
            @display("p=110.5,375.0625;i=device/server");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=60,60");
        }
        visualizer: IntegratedMultiCanvasVisualizer {
            @display("p=60,120");
        }
    connections:
        router1.ethg++ <--> Eth10G <--> router3.ethg++;
        router1.ethg++ <--> Eth10G <--> router2.ethg++;
        router3.ethg++ <--> Eth10G <--> router2.ethg++;
        router2.ethg++ <--> Eth100M <--> switch2.ethg++;
        router3.ethg++ <--> Eth100M <--> switch3.ethg++;
        switch1.ethg++ <--> Eth100M <--> router1.ethg++;
        switch3.ethg++ <--> Eth100M <--> host1.ethg++;
        switch3.ethg++ <--> Eth100M <--> host2.ethg++;
        switch3.ethg++ <--> Eth100M <--> host3.ethg++;
        switch2.ethg++ <--> Eth100M <--> host4.ethg++;
        switch2.ethg++ <--> Eth100M <--> host5.ethg++;
        switch1.ethg++ <--> Eth100M <--> server.ethg++;
}
