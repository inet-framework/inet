// SPDX-License-Identifier: LGPL-3.0-or-later
package inet.examples.mqttsn;
import inet.networks.base.WiredNetworkBase;
import inet.node.inet.StandardHost;
import inet.node.ethernet.EthernetSwitch;
import inet.node.ethernet.EthernetLink;
import inet.common.scenario.ScenarioManager;

network MqttSnNetwork extends WiredNetworkBase
{
    parameters:
        visualizer.typename = default("");
    submodules:
        sensor: StandardHost { @display("p=150,100"); }
        gateway: StandardHost { @display("p=150,250"); }
        broker: StandardHost { @display("p=550,100"); }
        observer: StandardHost { @display("p=550,250"); }
        switch: EthernetSwitch { @display("p=350,175"); }
        scenarioManager: ScenarioManager { @display("p=100,400"); }
    connections:
        sensor.ethg++ <--> EthernetLink { datarate = 100Mbps; delay = 1ms; } <--> switch.ethg++;
        gateway.ethg++ <--> EthernetLink { datarate = 100Mbps; delay = 1ms; } <--> switch.ethg++;
        broker.ethg++ <--> EthernetLink { datarate = 100Mbps; delay = 1ms; } <--> switch.ethg++;
        observer.ethg++ <--> EthernetLink { datarate = 100Mbps; delay = 1ms; } <--> switch.ethg++;
}
