// SPDX-License-Identifier: LGPL-3.0-or-later
package inet.examples.mqtt;
import inet.networks.base.WiredNetworkBase;
import inet.node.inet.StandardHost;
import inet.node.ethernet.EthernetSwitch;
import inet.node.ethernet.EthernetLink;
import inet.common.scenario.ScenarioManager;

// Publisher and subscriber use independent TCP connections to one broker.
network MqttNetwork extends WiredNetworkBase
{
    parameters:
        visualizer.typename = default("");
    submodules:
        publisher: StandardHost { @display("p=150,150"); }
        subscriber: StandardHost { @display("p=150,300"); }
        broker: StandardHost { @display("p=550,225"); }
        switch: EthernetSwitch { @display("p=350,225"); }
        scenarioManager: ScenarioManager { @display("p=100,500"); }
    connections:
        publisher.ethg++ <--> EthernetLink { datarate = 100Mbps; delay = 1ms; } <--> switch.ethg++;
        subscriber.ethg++ <--> EthernetLink { datarate = 100Mbps; delay = 1ms; } <--> switch.ethg++;
        broker.ethg++ <--> EthernetLink { datarate = 100Mbps; delay = 1ms; } <--> switch.ethg++;
}
