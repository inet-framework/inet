// SPDX-License-Identifier: LGPL-3.0-or-later
package inet.tests.mqtt;
import inet.networks.base.WiredNetworkBase;
import inet.node.inet.StandardHost;
import inet.node.ethernet.EthernetSwitch;
import inet.node.ethernet.EthernetLink;

network MqttTestNetwork extends WiredNetworkBase
{
    parameters:
        visualizer.typename = default("");
    submodules:
        publisher: StandardHost { @display("p=150,100"); }
        subscriber: StandardHost { @display("p=150,250"); }
        observer: StandardHost { @display("p=150,400"); }
        broker: StandardHost { @display("p=550,250"); }
        switch: EthernetSwitch { @display("p=350,250"); }
    connections:
        publisher.ethg++ <--> EthernetLink { datarate = 100Mbps; delay = 1ms; } <--> switch.ethg++;
        subscriber.ethg++ <--> EthernetLink { datarate = 100Mbps; delay = 1ms; } <--> switch.ethg++;
        observer.ethg++ <--> EthernetLink { datarate = 100Mbps; delay = 1ms; } <--> switch.ethg++;
        broker.ethg++ <--> EthernetLink { datarate = 100Mbps; delay = 1ms; } <--> switch.ethg++;
}
