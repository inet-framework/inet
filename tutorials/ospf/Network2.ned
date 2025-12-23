package inet.tutorials.ospf;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.visualizer.integrated.IntegratedCanvasVisualizer;
import inet.common.scenario.ScenarioManager;
import inet.node.ethernet.Eth100M;
import inet.node.ethernet.Eth10M;
import ned.DatarateChannel;
import inet.node.ethernet.EtherSwitch;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;

network Network2
{
    @display("bgb=1869.6599,1536.9");

    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=186.12,53.579998");
        }
        visualizer: IntegratedCanvasVisualizer {
            @display("p=186.12,250.98");
        }
        scenarioManager: ScenarioManager {
            @display("p=186.12,451.19998");
        }
        host0: StandardHost {
            @display("p=296.1,673.98");
        }
        host1: StandardHost {
            @display("p=296.1,831.89996");
        }
        host2: StandardHost {
            @display("p=296.1,992.63995");
        }
        R1: Router {
            @display("p=744.48,831.89996");
        }
        R2: Router {
            @display("p=1209.78,772.68");
        }
        host3: StandardHost {
            @display("p=1765.32,611.94");
        }
        host4: StandardHost {
            @display("p=1765.32,772.68");
        }
        host5: StandardHost {
            @display("p=1765.32,933.42");
        }
        switch1: EtherSwitch {
            @display("p=518.88,831.89996");
        }
        switch3: EtherSwitch {
            @display("p=1514.34,769.86");
        }
        switch2: EtherSwitch {
            @display("p=1001.1,798.06");
        }
        R3: Router {
            @display("p=916.5,532.98");
        }
        R4: Router {
            @display("p=1223.88,532.98");
        }
        R5: Router {
            @display("p=786.77997,1051.86");
        }
        R6: Router {
            @display("p=1159.02,1051.86");
        }
        host6: StandardHost {
            @display("p=659.88,183.3");
        }
        host7: StandardHost {
            @display("p=659.88,344.03998");
        }
        host8: StandardHost {
            @display("p=659.88,507.59998");
        }
        switch4: EtherSwitch {
            @display("p=882.66,344.03998");
        }
        host9: StandardHost {
            @display("p=1446.6599,188.94");
        }
        host10: StandardHost {
            @display("p=1446.6599,344.03998");
        }
        host11: StandardHost {
            @display("p=1446.6599,510.41998");
        }
        switch5: EtherSwitch {
            @display("p=1243.62,344.03998");
        }
        host12: StandardHost {
            @display("p=521.7,1096.98");
        }
        host13: StandardHost {
            @display("p=521.7,1257.72");
        }
        host14: StandardHost {
            @display("p=521.7,1418.46");
        }
        switch6: EtherSwitch {
            @display("p=744.48,1257.72");
        }
        host15: StandardHost {
            @display("p=1393.08,1096.98");
        }
        host16: StandardHost {
            @display("p=1393.08,1257.72");
        }
        host17: StandardHost {
            @display("p=1393.08,1418.46");
        }
        switch7: EtherSwitch {
            @display("p=1187.22,1257.72");
        }
    connections:
        host2.ethg++ <--> Eth100M <--> switch1.ethg++;
        host1.ethg++ <--> Eth100M <--> switch1.ethg++;
        host0.ethg++ <--> Eth100M <--> switch1.ethg++;
        switch1.ethg++ <--> Eth100M <--> R1.ethg++;

        switch3.ethg++ <--> Eth100M <--> host3.ethg++;
        switch3.ethg++ <--> Eth100M <--> host4.ethg++;
        switch3.ethg++ <--> Eth100M <--> host5.ethg++;

        R2.ethg++ <--> Eth100M <--> switch3.ethg++;
        R1.ethg++ <--> Eth100M <--> switch2.ethg++;
        R3.ethg++ <--> Eth100M <--> switch2.ethg++;
        R4.ethg++ <--> Eth100M <--> switch2.ethg++;
        R2.ethg++ <--> Eth100M <--> switch2.ethg++;
        R6.ethg++ <--> Eth100M <--> switch2.ethg++;
        R5.ethg++ <--> Eth100M <--> switch2.ethg++;
        host7.ethg++ <--> Eth100M <--> switch4.ethg++;
        host8.ethg++ <--> Eth100M <--> switch4.ethg++;
        host6.ethg++ <--> Eth100M <--> switch4.ethg++;
        host10.ethg++ <--> Eth100M <--> switch5.ethg++;
        host11.ethg++ <--> Eth100M <--> switch5.ethg++;
        host9.ethg++ <--> Eth100M <--> switch5.ethg++;
        host13.ethg++ <--> Eth100M <--> switch6.ethg++;
        host14.ethg++ <--> Eth100M <--> switch6.ethg++;
        host12.ethg++ <--> Eth100M <--> switch6.ethg++;
        host16.ethg++ <--> Eth100M <--> switch7.ethg++;
        host17.ethg++ <--> Eth100M <--> switch7.ethg++;
        host15.ethg++ <--> Eth100M <--> switch7.ethg++;
        R5.ethg++ <--> Eth100M <--> switch6.ethg++;
        R6.ethg++ <--> Eth100M <--> switch7.ethg++;
        switch4.ethg++ <--> Eth100M <--> R3.ethg++;
        switch5.ethg++ <--> Eth100M <--> R4.ethg++;
}

