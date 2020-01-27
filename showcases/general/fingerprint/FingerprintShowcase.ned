package inet.showcases.general.fingerprint;

import inet.common.scenario.ScenarioManager;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ethernet.Eth100M;
import inet.node.ethernet.Eth10G;
import inet.node.ethernet.EtherSwitch;
import inet.node.inet.AdhocHost;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import inet.physicallayer.contract.packetlevel.IRadioMedium;
import inet.physicallayer.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.visualizer.integrated.IntegratedCanvasVisualizer;
import inet.visualizer.integrated.IntegratedMultiCanvasVisualizer;
import inet.visualizer.integrated.IntegratedMultiVisualizer;
import inet.visualizer.integrated.IntegratedVisualizer;
import ned.DatarateChannel;


network FingerprintShowcaseWireless
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

network FingerprintShowcaseMixed
{
    @display("bgb=1214,400");
    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=87,57");
        }
        visualizer: IntegratedMultiVisualizer {
            @display("p=87,130");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            @display("p=86,192");
        }
        host1: AdhocHost {
            @display("p=201,243");
        }
        host2: AdhocHost {
            @display("p=289,115");
        }
        ethHost1: StandardHost {
            @display("p=405,176");
        }
        ethHost2: StandardHost {
            @display("p=1126,176");
        }
        router1: Router {
            @display("p=674,176");
        }
        router2: Router {
            @display("p=891,176");
        }
    connections:
        ethHost1.ethg++ <--> Eth100M <--> router1.ethg++;
        router2.ethg++ <--> Eth100M <--> ethHost2.ethg++;
        router1.pppg++ <--> Eth100M <--> router2.pppg++;
}

network FingerprintShowcaseWired
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
        switch2: EtherSwitch {
            @display("p=643.875,374");
        }
        switch3: EtherSwitch {
            @display("p=427.125,167.875");
        }
        switch1: EtherSwitch {
            @display("p=210.375,374");
        }
        host4: StandardHost {
            @display("p=745.875,308.125");
        }
        host5: StandardHost {
            @display("p=745.875,434.5625");
            @statistic[rcvdBytes](title="bytes received"; source="sum(packetBytes(app[0].packetReceived))"; record=last; checkSignals=false);
        }
        host1: StandardHost {
            @display("p=365.5,78.625");
        }
        host2: StandardHost {
            @display("p=522.75,78.625");
        }
        host3: StandardHost {
            @display("p=574.8125,168.9375");
            @statistic[rcvdBytes](title="bytes received"; source="sum(packetBytes(app[0].packetReceived))"; record=last; checkSignals=false);
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

network OspfNetwork
{
    @display("bgb=1766.2515,641.51587");

    types:
        channel PppLink100M extends DatarateChannel
        {
            delay = 5us;
            datarate = 100Mbps;
        }

    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=491.625,81.225");
        }
        visualizer: IntegratedCanvasVisualizer {
            @display("p=756.675,81.225");
        }
        scenarioManager: ScenarioManager {
            @display("p=1045.2375,81.225");
        }
        host0: StandardHost {
            @display("p=98.325,200.925");
        }
        host1: StandardHost {
            @display("p=98.325,356.9625");
        }
        host2: StandardHost {
            @display("p=98.325,519.4125");
        }
        R1: Router {
            @display("p=545.0625,356.9625");
        }
        R2: Router {
            @display("p=989.6625,356.9625");
        }
        host3: StandardHost {
            @display("p=1545.4125,196.65");
        }
        host4: StandardHost {
            @display("p=1545.4125,356.9625");
        }
        host5: StandardHost {
            @display("p=1545.4125,517.275");
        }
        switch1: EtherSwitch {
            @display("p=320.625,356.9625");
        }
        switch2: EtherSwitch {
            @display("p=1295.325,352.6875");
        }
    connections:
        host2.ethg++ <--> Eth100M <--> switch1.ethg++;
        host1.ethg++ <--> Eth100M <--> switch1.ethg++;
        host0.ethg++ <--> Eth100M <--> switch1.ethg++;
        switch1.ethg++ <--> Eth100M <--> R1.ethg++;

        switch2.ethg++ <--> Eth100M <--> host3.ethg++;
        switch2.ethg++ <--> Eth100M <--> host4.ethg++;
        switch2.ethg++ <--> Eth100M <--> host5.ethg++;

        R1.pppg++ <--> PppLink100M <--> R2.pppg++;
        R2.ethg++ <--> PppLink100M <--> switch2.ethg++;
}
