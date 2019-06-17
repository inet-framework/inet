
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

network OspfNetwork
{
    @display("bgb=2099.7375,852.525");

    types:
        channel PppLink100M extends DatarateChannel
        {
            delay = 5us;
            datarate = 100Mbps;
        }

        channel PppLink10M extends DatarateChannel
        {
            delay = 5us;
            datarate = 10Mbps;
        }

    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=757.8,76.30625");
        }
        visualizer: IntegratedCanvasVisualizer {
            @display("p=1023.5563,76.30625");
        }
        scenarioManager: ScenarioManager {
            @display("p=1310.3625,76.30625");
        }
        host0: StandardHost {
            @display("p=97.81249,375.59998");
        }
        host1: StandardHost {
            @display("p=97.81249,532.1");
        }
        host2: StandardHost {
            @display("p=97.81249,694.46875");
        }
        host3: StandardHost {
            @display("p=1271.5625,334.51874");
        }
        R1: Router {
            @display("p=543.83746,532.1");
        }
        R3: Router {
            @display("p=1508.2687,532.1");
        }
        R2: Router {
            @display("p=925.3062,532.1");
        }
        host4: StandardHost {
            @display("p=1271.5625,692.51245");
        }
        host5: StandardHost {
            @display("p=1473.0562,389.29373");
        }
        host6: StandardHost {
            @display("p=1977.7687,373.64374");
        }
        host7: StandardHost {
            @display("p=1977.7687,532.1");
        }
        host8: StandardHost {
            @display("p=1977.7687,692.51245");
        }
        switch0: EtherSwitch {
            @display("p=318.86874,530.14374");
        }
        switch2: EtherSwitch {
            @display("p=1727.3687,530.14374");
        }
        switch1: EtherSwitch {
            @display("p=1271.5625,530.14374");
        }
        R4: Router {
            @display("p=1054.4187,334.51874");
        }
        R5: Router {
            @display("p=712.07495,334.51874");
        }

    connections:
        host2.ethg++ <--> Eth100M <--> switch0.ethg++;
        host1.ethg++ <--> Eth100M <--> switch0.ethg++;
        host0.ethg++ <--> Eth100M <--> switch0.ethg++;
        switch0.ethg++ <--> Eth100M <--> R1.ethg++;

        switch1.ethg++ <--> Eth100M <--> host3.ethg++;
        switch1.ethg++ <--> Eth100M <--> host4.ethg++;
        switch1.ethg++ <--> Eth100M <--> host5.ethg++;
        switch1.ethg++ <--> Eth100M <--> R3.ethg++;

        switch2.ethg++ <--> Eth100M <--> host6.ethg++;
        switch2.ethg++ <--> Eth100M <--> host7.ethg++;
        switch2.ethg++ <--> Eth100M <--> host8.ethg++;

        R3.ethg++ <--> Eth100M <--> switch2.ethg++;
        R1.pppg++ <--> PppLink10M <--> R2.pppg++;
        R2.ethg++ <--> Eth10M <--> switch1.ethg++;
        R1.pppg++ <--> PppLink100M <--> R5.pppg++;
        R5.pppg++ <--> PppLink100M <--> R4.pppg++;
        R4.ethg++ <--> Eth100M <--> switch1.ethg++;
}


network Network
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


network RouterLSA
{
    @display("bgb=1950.5325,964.7512");

    types:
        channel PppLink100M extends DatarateChannel
        {
            delay = 5us;
            datarate = 100Mbps;
        }

        channel PppLink10M extends DatarateChannel
        {
            delay = 5us;
            datarate = 10Mbps;
        }

    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=1222.3688,86.74875");
        }
        visualizer: IntegratedCanvasVisualizer {
            @display("p=1461.585,86.74875");
        }
        scenarioManager: ScenarioManager {
            @display("p=1748.1188,86.74875");
        }
        host0: StandardHost {
            @display("p=144.58125,486.31876");
        }
        host1: StandardHost {
            @display("p=144.58125,646.6725");
        }
        host2: StandardHost {
            @display("p=144.58125,809.65497");
        }
        host3: StandardHost {
            @display("p=1069.9012,449.51624");
        }
        R1: Router {
            @display("p=586.21124,646.6725");
        }
        R2: Router {
            @display("p=1306.4888,646.6725");
        }
        host4: StandardHost {
            @display("p=1069.9012,807.02625");
        }
        host5: StandardHost {
            @display("p=1269.6863,504.72");
        }
        host6: StandardHost {
            @display("p=1774.4062,486.31876");
        }
        host7: StandardHost {
            @display("p=1774.4062,649.3013");
        }
        host8: StandardHost {
            @display("p=1774.4062,809.65497");
        }
        switch0: EtherSwitch {
            @display("p=365.39624,644.04376");
        }
        switch2: EtherSwitch {
            @display("p=1524.6749,644.04376");
        }
        switch1: EtherSwitch {
            @display("p=1069.9012,641.415");
        }
        R4: Router {
            @display("p=854.34375,449.51624");
        }
        R3: Router {
            @display("p=586.21124,449.51624");
        }

        host9: StandardHost {
            @display("p=775.48126,254.98875");
        }
        host10: StandardHost {
            @display("p=586.21124,86.74875");
        }
        host11: StandardHost {
            @display("p=365.39624,254.98875");
        }
        switch3: EtherSwitch {
            @display("p=586.21124,252.36");
        }
    connections:
        host2.ethg++ <--> Eth100M <--> switch0.ethg++;
        host1.ethg++ <--> Eth100M <--> switch0.ethg++;
        host0.ethg++ <--> Eth100M <--> switch0.ethg++;
        switch0.ethg++ <--> Eth100M <--> R1.ethg++;

        switch1.ethg++ <--> Eth100M <--> host3.ethg++;
        switch1.ethg++ <--> Eth100M <--> host4.ethg++;
        switch1.ethg++ <--> Eth100M <--> host5.ethg++;
        switch1.ethg++ <--> Eth100M <--> R2.ethg++;

        switch2.ethg++ <--> Eth100M <--> host6.ethg++;
        switch2.ethg++ <--> Eth100M <--> host7.ethg++;
        switch2.ethg++ <--> Eth100M <--> host8.ethg++;

        R2.ethg++ <--> Eth100M <--> switch2.ethg++;
        R1.pppg++ <--> PppLink100M <--> R3.pppg++;
        R4.ethg++ <--> Eth100M <--> switch1.ethg++;
        R1.ethg++ <--> Eth100M <--> switch1.ethg++;
        host10.ethg++ <--> Eth100M <--> switch3.ethg++;
        host11.ethg++ <--> Eth100M <--> switch3.ethg++;
        host9.ethg++ <--> Eth100M <--> switch3.ethg++;
        R3.ethg++ <--> Eth100M <--> switch3.ethg++;
}


network InterfaceNetworkType
{
    @display("bgb=2331.1575,1232.7675");

    types:
        channel PppLink100M extends DatarateChannel
        {
            delay = 5us;
            datarate = 100Mbps;
        }

    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=867.888,89.376");
        }
        visualizer: IntegratedCanvasVisualizer {
            @display("p=1131.312,89.376");
        }
        scenarioManager: ScenarioManager {
            @display("p=1420.608,89.376");
        }
        host0: StandardHost {
            @display("p=96.31875,198.475");
        }
        host1: StandardHost {
            @display("p=96.401245,359.31375");
        }
        host2: StandardHost {
            @display("p=96.31875,539.96875");
        }
        R1: Router {
            @display("p=545.0625,356.9625");
        }
        R2: Router {
            @display("p=1001.952,359.85602");
        }
        R3: Router {
            @display("p=1453.536,357.504");
        }
        host3: StandardHost {
            @display("p=2095.632,199.92");
        }
        host4: StandardHost {
            @display("p=2095.632,359.85602");
        }
        host5: StandardHost {
            @display("p=2095.632,519.792");
        }
        switch1: EtherSwitch {
            @display("p=320.625,356.9625");
        }
        switch2: EtherSwitch {
            @display("p=1843.968,355.152");
        }
        host6: StandardHost {
            @display("p=96.401245,727.39124");
        }
        host7: StandardHost {
            @display("p=96.401245,885.13873");
        }
        host8: StandardHost {
            @display("p=96.401245,1095.4688");
        }
        R4: Router {
            @display("p=543.3525,885.13873");
        }
        R5: Router {
            @display("p=1001.952,886.704");
        }
        R6: Router {
            @display("p=1451.8612,885.13873");
        }
        host9: StandardHost {
            @display("p=2094.5361,712.785");
        }
        host10: StandardHost {
            @display("p=2094.5361,902.66626");
        }
        host11: StandardHost {
            @display("p=2094.5361,1095.4688");
        }
        switch3: EtherSwitch {
            @display("p=1843.3087,882.21747");
        }
        switch4: EtherSwitch {
            @display("p=318.41623,882.21747");
        }
    connections:
        host1.ethg++ <--> Eth100M <--> switch1.ethg++;
        host0.ethg++ <--> Eth100M <--> switch1.ethg++;
        host2.ethg++ <--> Eth100M <--> switch1.ethg++;
        switch1.ethg++ <--> Eth100M <--> R1.ethg++;

        switch2.ethg++ <--> Eth100M <--> host3.ethg++;
        switch2.ethg++ <--> Eth100M <--> host4.ethg++;
        switch2.ethg++ <--> Eth100M <--> host5.ethg++;

        R3.ethg++ <--> Eth100M <--> switch2.ethg++;
        host9.ethg++ <--> Eth100M <--> switch3.ethg++;
        host10.ethg++ <--> Eth100M <--> switch3.ethg++;
        host11.ethg++ <--> Eth100M <--> switch3.ethg++;
        switch3.ethg++ <--> Eth100M <--> R6.ethg++;

        switch4.ethg++ <--> Eth100M <--> host6.ethg++;
        switch4.ethg++ <--> Eth100M <--> host7.ethg++;
        switch4.ethg++ <--> Eth100M <--> host8.ethg++;

        R4.ethg++ <--> Eth100M <--> switch4.ethg++;
        R1.pppg++ <--> PppLink100M <--> R2.pppg++;
        R2.pppg++ <--> PppLink100M <--> R3.pppg++;
        R6.ethg++ <--> Eth100M <--> R5.ethg++;
        R5.ethg++ <--> Eth100M <--> R4.ethg++;
}


network topologyChange
{
    @display("bgb=1914.2025,894.73376");

    types:
        channel PppLink100M extends DatarateChannel
        {
            delay = 5us;
            datarate = 100Mbps;
        }

        channel PppLink10M extends DatarateChannel
        {
            delay = 5us;
            datarate = 10Mbps;
        }

    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=652.46,105.545");
        }
        visualizer: IntegratedCanvasVisualizer {
            @display("p=918.72125,105.545");
        }
        scenarioManager: ScenarioManager {
            @display("p=1204.1725,105.545");
        }
        R2: Router {
            @display("p=959.4937,573.0675");
        }
        switch3: EtherSwitch {
            @display("p=160.35374,570.4387");
        }
        R4: Router {
            @display("p=410.085,573.0675");
        }
        R3: Router {
            @display("p=709.7625,573.0675");
        }

        switch2: EtherSwitch {
            @display("p=1264.4287,570.4387");
        }
        switch5: EtherSwitch {
            @display("p=709.7625,378.54");
        }
        R1: Router {
            @display("p=1519.4175,573.0675");
        }
        switch4: EtherSwitch {
            @display("p=959.4937,378.54");
        }
        switch1: EtherSwitch {
            @display("p=1782.2925,567.81");
        }
        R5: Router {
            @display("p=1264.1412,379.0025");
        }
        R6: Router {
            @display("p=1264.1412,801.1825");
        }
        switch6: EtherSwitch {
            @display("p=1517.6176,376.4075");
        }
        switch7: EtherSwitch {
            @display("p=1517.6176,798.3675");
        }
    connections:
        R2.pppg++ <--> PppLink100M <--> R3.pppg++;
        R3.pppg++ <--> PppLink100M <--> R4.pppg++;
        R4.ethg++ <--> Eth100M <--> switch3.ethg++;
        R3.ethg++ <--> Eth100M <--> switch5.ethg++;
        R2.ethg++ <--> Eth100M <--> switch2.ethg++;
        R1.ethg++ <--> Eth100M <--> switch2.ethg++;
        switch4.ethg++ <--> Eth100M <--> R2.ethg++;
        switch1.ethg++ <--> Eth100M <--> R1.ethg++;
        switch2.ethg++ <--> Eth100M <--> R5.ethg++;
        R6.ethg++ <--> Eth100M <--> switch2.ethg++;
        switch6.ethg++ <--> Eth100M <--> R5.ethg++;
        switch7.ethg++ <--> Eth100M <--> R6.ethg++;
}


network freshness
{
    @display("bgb=1154.256,1107.552");

    types:
        channel PppLink100M extends DatarateChannel
        {
            delay = 5us;
            datarate = 100Mbps;
        }

        channel PppLink10M extends DatarateChannel
        {
            delay = 5us;
            datarate = 10Mbps;
        }

    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=197.936,135.664");
        }
        visualizer: IntegratedCanvasVisualizer {
            @display("p=464.816,135.664");
        }
        scenarioManager: ScenarioManager {
            @display("p=749.48804,135.664");
        }
        R1: Router {
            @display("p=998.57605,535.984");
        }
        R5: Router {
            @display("p=749.48804,760.60803");
        }
        R6: Router {
            @display("p=998.57605,760.60803");
        }
        switch4: EtherSwitch {
            @display("p=449.24802,342.496");
        }
        switch5: EtherSwitch {
            @display("p=449.24802,962.992");
        }
        R3: Router {
            @display("p=449.24802,535.984");
        }
        R2: Router {
            @display("p=749.48804,535.984");
        }

        R4: Router {
            @display("p=449.24802,760.60803");
        }
        switch6: EtherSwitch {
            @display("p=749.48804,962.992");
        }
        switch7: EtherSwitch {
            @display("p=998.57605,962.992");
        }
        switch3: EtherSwitch {
            @display("p=749.48804,342.496");
        }
        switch2: EtherSwitch {
            @display("p=998.57605,342.496");
        }
        R7: Router {
            @display("p=171.248,760.60803");
        }
        switch1: EtherSwitch {
            @display("p=171.248,962.992");
        }
    connections:
        R1.pppg++ <--> PppLink100M <--> R2.pppg++;
        R2.pppg++ <--> PppLink100M <--> R3.pppg++;
        R4.pppg++ <--> PppLink100M <--> R5.pppg++;
        R5.pppg++ <--> PppLink100M <--> R6.pppg++;
        R6.pppg++ <--> PppLink100M <--> R1.pppg++;
        R3.pppg++ <--> PppLink100M <--> R4.pppg++;
        R3.ethg++ <--> Eth100M <--> switch4.ethg++;
        R4.ethg++ <--> Eth100M <--> switch5.ethg++;
        R5.ethg++ <--> Eth100M <--> switch6.ethg++;
        R6.ethg++ <--> Eth100M <--> switch7.ethg++;
        R2.ethg++ <--> Eth100M <--> switch3.ethg++;
        switch2.ethg++ <--> Eth100M <--> R1.ethg++;
        R7.ethg++ <--> Eth100M <--> switch1.ethg++;
        R7.pppg++ <--> PppLink100M <--> R4.pppg++;
}


network OSPF_AreaTest
{
    @display("bgb=1153.1,510.9");

    types:
        channel PppLink100M extends DatarateChannel
        {
            delay = 5us;
            datarate = 100Mbps;
        }

    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=760.5,65");
        }
        visualizer: IntegratedCanvasVisualizer {
            @display("p=893.1,65");
        }
        scenarioManager: ScenarioManager {
            @display("p=1055.6,65");
        }
        host1: StandardHost {
            @display("p=102.7,67.6");
        }
        host2: StandardHost {
            @display("p=92.299995,445.9");
        }
        R1: Router {
            @display("p=392.6,157.3");
        }
        R2: Router {
            @display("p=392.6,336.69998");
        }
        R3: Router {
            @display("p=542.1,245.7");
        }
        switch1: EtherSwitch {
            @display("p=243.09999,66.299995");
        }
        switch2: EtherSwitch {
            @display("p=243.09999,445.9");
        }
        host3: StandardHost {
            @display("p=1085.5,245.7");
        }
        R4: Router {
            @display("p=678.6,245.7");
        }
        R5: Router {
            @display("p=821.6,245.7");
        }
        switch3: EtherSwitch {
            @display("p=955.5,244.4");
        }

    connections:
        host1.ethg++ <--> Eth100M <--> switch1.ethg++;
        switch1.ethg++ <--> Eth100M <--> R1.ethg++;
        R1.pppg++ <--> PppLink100M <--> R3.pppg++;
        switch2.ethg++ <--> Eth100M <--> host2.ethg++;
        R1.pppg++ <--> PppLink100M <--> R2.pppg++;
        R2.pppg++ <--> PppLink100M <--> R3.pppg++;
        R2.ethg++ <--> Eth100M <--> switch2.ethg++;
        R4.pppg++ <--> PppLink100M <--> R5.pppg++;
        R5.ethg++ <--> Eth100M <--> switch3.ethg++;
        switch3.ethg++ <--> Eth100M <--> host3.ethg++;
        R3.pppg++ <--> PppLink100M <--> R4.pppg++;
}


network OSPF_LoopAvoidance
{
    @display("bgb=692,488");

    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=62.18875,63.635002");
        }
        visualizer: IntegratedCanvasVisualizer {
            @display("p=195.24374,63.635002");
        }
        scenarioManager: ScenarioManager {
            @display("p=357.22375,63.635002");
        }
        R1: Router {
            @display("p=218,179");
        }
        R2: Router {
            @display("p=474,179");
        }
        R3: Router {
            @display("p=349,297");
        }
        R4: Router {
            @display("p=474,411");
        }
        R5: Router {
            @display("p=218,411");
        }
        switch1: EtherSwitch {
            @display("p=72,177");
        }
        switch2: EtherSwitch {
            @display("p=619,177");
        }
        switch3: EtherSwitch {
            @display("p=250,296");
        }
        switch4: EtherSwitch {
            @display("p=619,410");
        }
        switch5: EtherSwitch {
            @display("p=72,410");
        }
        switch6: EtherSwitch {
            @display("p=349,410");
        }

    connections:
        switch1.ethg++ <--> Eth100M <--> R1.ethg++;
        R2.ethg++ <--> Eth100M <--> R3.ethg++;
        R2.ethg++ <--> Eth100M <--> switch2.ethg++;
        R5.ethg++ <--> Eth100M <--> switch5.ethg++;
        R3.ethg++ <--> Eth100M <--> R4.ethg++;
        R3.ethg++ <--> Eth100M <--> R5.ethg++;
        R5.ethg++ <--> Eth100M <--> R1.ethg++;
        R4.ethg++ <--> Eth100M <--> R2.ethg++;
        switch4.ethg++ <--> Eth100M <--> R4.ethg++;
        R1.ethg++ <--> Eth100M <--> R2.ethg++;
        switch3.ethg++ <--> Eth100M <--> R3.ethg++;
        switch6.ethg++ <--> Eth100M <--> R3.ethg++;
}


network OSPF_Summary_LSA
{
    @display("bgb=1642.8824,629.9775");

    types:
        channel PppLink100M extends DatarateChannel
        {
            delay = 5us;
            datarate = 100Mbps;
        }

        channel PppLink10M extends DatarateChannel
        {
            delay = 5us;
            datarate = 10Mbps;
        }

    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=210.055,102.585");
        }
        visualizer: IntegratedCanvasVisualizer {
            @display("p=473.845,102.585");
        }
        scenarioManager: ScenarioManager {
            @display("p=744.96246,102.585");
        }
        R1: Router {
            @display("p=564.0975,282.04874");
        }
        R2: Router {
            @display("p=174.99374,282.04874");
        }
        R3: Router {
            @display("p=1472.0062,282.04874");
        }
        R4: Router {
            @display("p=1084.9613,282.04874");
        }
        R1A: Router {
            @display("p=564.0975,516.7463");
        }
        R2A: Router {
            @display("p=174.99374,516.7463");
        }
        R3A: Router {
            @display("p=1472.0062,516.7463");
        }
        R4A: Router {
            @display("p=1084.9613,516.7463");
        }
    connections:
        R1.ethg++ <--> Eth100M <--> R2.ethg++;
        R1.ethg++ <--> Eth100M <--> R4.ethg++;
        R4.ethg++ <--> Eth100M <--> R3.ethg++;
        R2A.ethg++ <--> Eth100M <--> R2.ethg++;
        R1A.ethg++ <--> Eth100M <--> R1.ethg++;
        R4A.ethg++ <--> Eth100M <--> R4.ethg++;
        R3A.ethg++ <--> Eth100M <--> R3.ethg++;
}


module OspfLan
{
    parameters:
        int h; // number of hosts on the hub
        @display("i=old/cloud");
    gates:
        inout ethg[];
    submodules:
        switch: EtherSwitch;
        host[h]: StandardHost {
            @display("p=199,159");
        }
        visualizer: IntegratedCanvasVisualizer {
            @display("p=750.1,49.399998");
        }
    connections:
        for i=0..sizeof(ethg)-1 {
            switch.ethg++ <--> ethg[i];
        }
        for i=0..h-1 {
            switch.ethg++ <--> Eth100M <--> host[i].ethg++;
        }
}

network OSPF_Area_External extends OSPF_AreaTest
{
    @display("bgb=1151.665,517.31");
    submodules:
        N1: OspfLan {
            parameters:
                h = 1;
                @display("p=539.45123,389.04126");
        }
        N2: OspfLan {
            parameters:
                h = 1;
                @display("p=820.02374,389.04126");
        }

    connections:
        R3.ethg++ <--> Eth100M <--> N1.ethg++;
        R5.ethg++ <--> Eth100M <--> N2.ethg++;
}


network OSPF_Area_External_Forwarding
{
    @display("bgb=1064.7,561.6");
    types:
        channel PppLink100M extends DatarateChannel
        {
            delay = 5us;
            datarate = 100Mbps;
        }

    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=618.8,49.399998");
        }
        visualizer: IntegratedCanvasVisualizer {
            @display("p=750.1,49.399998");
        }
        scenarioManager: ScenarioManager {
            @display("p=913.89996,49.399998");
        }
        host1: StandardHost {
            @display("p=92.299995,120.9");
        }
        host2: StandardHost {
            @display("p=92.299995,500.5");
        }
        R1: Router {
            @display("p=382.19998,210.59999");
        }
        R2: Router {
            @display("p=382.19998,390");
        }
        R3: Router {
            @display("p=531.7,299");
        }
        switch1: EtherSwitch {
            @display("p=232.7,118.299995");
        }
        switch2: EtherSwitch {
            @display("p=232.7,499.19998");
        }
        R4: Router {
            @display("p=683.8,299");
        }
        R5: Router {
            @display("p=845,410.8");
        }
        R6: Router {
            @display("p=845,183.3");
        }
        switch3: EtherSwitch {
            @display("p=845,297.69998");
        }
        host3: StandardHost {
            @display("p=683.8,183.3");
        }

    connections:
        host1.ethg++ <--> Eth100M <--> switch1.ethg++;
        switch1.ethg++ <--> Eth100M <--> R1.ethg++;
        R1.pppg++ <--> PppLink100M <--> R3.pppg++;
        switch2.ethg++ <--> Eth100M <--> host2.ethg++;
        R1.pppg++ <--> PppLink100M <--> R2.pppg++;
        R2.pppg++ <--> PppLink100M <--> R3.pppg++;
        R2.ethg++ <--> Eth100M <--> switch2.ethg++;
        R5.ethg++ <--> Eth100M <--> switch3.ethg++;
        R3.pppg++ <--> PppLink100M <--> R4.pppg++;
        switch3.ethg++ <--> Eth100M <--> R4.ethg++;
        R6.ethg++ <--> Eth100M <--> switch3.ethg++;
        host3.ethg++ <--> Eth100M <--> R6.ethg++;
}


network OSPF_Default_Route_Distribution
{
    @display("bgb=1153.1,510.9");

    types:
        channel PppLink100M extends DatarateChannel
        {
            delay = 5us;
            datarate = 100Mbps;
        }

    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=760.5,65");
        }
        visualizer: IntegratedCanvasVisualizer {
            @display("p=893.1,65");
        }
        scenarioManager: ScenarioManager {
            @display("p=1055.6,65");
        }
        host1: StandardHost {
            @display("p=102.7,67.6");
        }
        host2: StandardHost {
            @display("p=92.299995,445.9");
        }
        R1: Router {
            @display("p=392.6,157.3");
        }
        R2: Router {
            @display("p=392.6,336.69998");
        }
        R3: Router {
            @display("p=542.1,245.7");
        }
        switch1: EtherSwitch {
            @display("p=243.09999,66.299995");
        }
        switch2: EtherSwitch {
            @display("p=243.09999,445.9");
        }
        host3: StandardHost {
            @display("p=1085.5,245.7");
        }
        R4: Router {
            @display("p=678.6,245.7");
        }
        R5: Router {
            @display("p=821.6,245.7");
        }
        switch3: EtherSwitch {
            @display("p=955.5,244.4");
        }
        N1: OspfLan {
            parameters:
                h = 1;
                @display("p=539.45123,389.04126");
        }

    connections:
        host1.ethg++ <--> Eth100M <--> switch1.ethg++;
        switch1.ethg++ <--> Eth100M <--> R1.ethg++;
        R1.pppg++ <--> PppLink100M <--> R3.pppg++;
        switch2.ethg++ <--> Eth100M <--> host2.ethg++;
        R1.pppg++ <--> PppLink100M <--> R2.pppg++;
        R2.pppg++ <--> PppLink100M <--> R3.pppg++;
        R2.ethg++ <--> Eth100M <--> switch2.ethg++;
        R4.pppg++ <--> PppLink100M <--> R5.pppg++;
        R5.ethg++ <--> Eth100M <--> switch3.ethg++;
        switch3.ethg++ <--> Eth100M <--> host3.ethg++;
        R3.pppg++ <--> PppLink100M <--> R4.pppg++;
        R3.ethg++ <--> Eth100M <--> N1.ethg++;
}


network OSPF_Stub extends OSPF_AreaTest
{
    @display("bgb=1151.665,517.31");
    submodules:
        N1: OspfLan {
            parameters:
                h = 1;
                @display("p=470.66248,443.23123");
        }
        N2: OspfLan {
            parameters:
                h = 1;
                @display("p=677.1187,443.23123");
        }
        N3: OspfLan {
            parameters:
                h = 1;
                @display("p=847.4812,444.675");
        }

    connections:
        R2.ethg++ <--> Eth100M <--> N1.ethg++;
        R3.ethg++ <--> Eth100M <--> N2.ethg++;
        R4.ethg++ <--> Eth100M <--> N3.ethg++;
}


network VirtualLink
{
    @display("bgb=1529.2438,788.60626");

    types:
        channel PppLink100M extends DatarateChannel
        {
            delay = 5us;
            datarate = 100Mbps;
        }

    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=136.23125,61.399998");
        }
        visualizer: IntegratedCanvasVisualizer {
            @display("p=136.23125,197.63124");
        }
        scenarioManager: ScenarioManager {
            @display("p=379.9125,61.399998");
        }
        R1: Router {
            @display("p=176.525,401.01874");
        }
        R2: Router {
            @display("p=483.525,401.01874");
        }
        R3: Router {
            @display("p=1076.4187,401.01874");
        }
        R4: Router {
            @display("p=1377.6625,401.01874");
        }
        switch1: EtherSwitch {
            @display("p=176.525,583.3");
        }
        switch2: EtherSwitch {
            @display("p=483.525,583.3");
        }
        switch3: EtherSwitch {
            @display("p=1076.4187,583.3");
        }
        switch4: EtherSwitch {
            @display("p=1377.6625,583.3");
        }
        R5: Router {
            @display("p=757.90625,401.01874");
        }
        switch5: EtherSwitch {
            @display("p=757.90625,583.3");
        }
        switch6: EtherSwitch {
            @display("p=755.9875,209.14375");
        }
    connections:
        R2.pppg++ <--> PppLink100M <--> R1.pppg++;
        R3.pppg++ <--> PppLink100M <--> R4.pppg++;
        R1.ethg++ <--> Eth100M <--> switch1.ethg++;
        R2.ethg++ <--> Eth100M <--> switch2.ethg++;
        R3.ethg++ <--> Eth100M <--> switch3.ethg++;
        R4.ethg++ <--> Eth100M <--> switch4.ethg++;
        R5.ethg++ <--> Eth100M <--> switch6.ethg++;
        R5.ethg++ <--> Eth100M <--> switch5.ethg++;
        R2.pppg++ <--> PppLink100M <--> R5.pppg++;
        R5.pppg++ <--> PppLink100M <--> R3.pppg++;
}


network VirtualLink_2
{
    @display("bgb=1421.8575,584.4563");

    types:
        channel PppLink100M extends DatarateChannel
        {
            delay = 5us;
            datarate = 100Mbps;
        }

    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=136.23125,61.399998");
        }
        visualizer: IntegratedCanvasVisualizer {
            @display("p=136.05376,208.87126");
        }
        scenarioManager: ScenarioManager {
            @display("p=136.05376,360.255");
        }
        R3: Router {
            @display("p=703.2638,465.64877");
        }
        R1: Router {
            @display("p=904.47003,260.61002");
        }
        switch3: EtherSwitch {
            @display("p=448.40253,463.7325");
        }
        switch1: EtherSwitch {
            @display("p=904.47003,47.906254");
        }
        R2: Router {
            @display("p=1052.0212,465.64877");
        }
        switch2: EtherSwitch {
            @display("p=1306.8826,463.7325");
        }
    connections:
        R3.ethg++ <--> Eth100M <--> switch3.ethg++;
        R1.ethg++ <--> Eth100M <--> switch1.ethg++;
        R2.ethg++ <--> Eth100M <--> switch2.ethg++;
        R1.pppg++ <--> PppLink100M <--> R2.pppg++;
        R2.pppg++ <--> PppLink100M <--> R3.pppg++;
}


network OSPF_Route_Selection
{
    @display("bgb=506.016,497.98398");

    types:
        channel PppLink100M extends DatarateChannel
        {
            delay = 5us;
            datarate = 100Mbps;
        }

    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=65,38");
        }
        visualizer: IntegratedCanvasVisualizer {
            @display("p=187,38");
        }
        scenarioManager: ScenarioManager {
            @display("p=324,38");
        }
        R1: Router {
            @display("p=447.784,290.156");
        }
        R2: Router {
            @display("p=236.944,140.56");
        }
        R3: Router {
            @display("p=236.944,240.95999");
        }
        R4: Router {
            @display("p=236.944,343.36798");
        }
        R5: Router {
            @display("p=236.944,448.788");
        }
        switch2: EtherSwitch {
            @display("p=64.256,137.54799");
        }
        switch4: EtherSwitch {
            @display("p=64.256,340.356");
        }
        switch5: EtherSwitch {
            @display("p=64.256,445.776");
        }
        R6: Router {
            @display("p=64.256,240.95999");
        }
    connections:
        R2.pppg++ <--> PppLink100M <--> R1.pppg++;
        R1.pppg++ <--> PppLink100M <--> R3.pppg++;
        R1.pppg++ <--> PppLink100M <--> R4.pppg++;
        R1.pppg++ <--> PppLink100M <--> R5.pppg++;
        switch2.ethg++ <--> Eth100M <--> R2.ethg++;
        switch4.ethg++ <--> Eth100M <--> R4.ethg++;
        switch5.ethg++ <--> Eth100M <--> R5.ethg++;
        R3.pppg++ <--> PppLink100M <--> R6.pppg++;
}


network OSPF_Suboptimal
{
    @display("bgb=772,445");

    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=65,38");
        }
        visualizer: IntegratedCanvasVisualizer {
            @display("p=187,38");
        }
        scenarioManager: ScenarioManager {
            @display("p=324,38");
        }
        R1: Router {
            @display("p=64.128,139.278");
        }
        R2: Router {
            @display("p=230.45999,139.278");
        }
        R3: Router {
            @display("p=559.11597,139.278");
        }
        R4: Router {
            @display("p=323.646,242.484");
        }
        R5: Router {
            @display("p=474.948,242.484");
        }
        switch4: EtherSwitch {
            @display("p=64.128,386.772");
        }
        switch3: EtherSwitch {
            @display("p=474.948,330.66");
        }
        R6: Router {
            @display("p=230.45999,387.774");
        }
        R7: Router {
            @display("p=559.11597,387.774");
        }
        switch1: EtherSwitch {
            @display("p=708,138");
        }
        switch2: EtherSwitch {
            @display("p=323.646,330.66");
        }
        host0: StandardHost {
            @display("p=65,246");
        }
        host6: StandardHost {
            @display("p=708,387");
        }
    connections:
        R2.ethg++ <--> Eth100M <--> R1.ethg++;
        R2.ethg++ <--> Eth100M <--> R3.ethg++;
        R2.ethg++ <--> Eth100M <--> R6.ethg++;
        R2.ethg++ <--> Eth100M <--> R4.ethg++;
        R4.ethg++ <--> Eth100M <--> R5.ethg++;
        R5.ethg++ <--> Eth100M <--> R3.ethg++;
        R3.ethg++ <--> Eth100M <--> R7.ethg++;
        R7.ethg++ <--> Eth100M <--> R6.ethg++;
        switch4.ethg++ <--> Eth100M <--> R6.ethg++;
        switch1.ethg++ <--> Eth100M <--> R3.ethg++;
        switch3.ethg++ <--> Eth100M <--> R5.ethg++;
        switch2.ethg++ <--> Eth100M <--> R4.ethg++;
        host0.ethg++ <--> Eth100M <--> R1.ethg++;
        host6.ethg++ <--> Eth100M <--> R7.ethg++;
}
