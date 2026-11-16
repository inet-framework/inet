// SPDX-License-Identifier: LGPL-3.0-or-later
package inet.tests.mqttsn;
import inet.networks.base.WiredNetworkBase;
import inet.node.inet.StandardHost;
import inet.node.ethernet.EthernetSwitch;
import inet.node.ethernet.EthernetLink;

network MqttSnTestNetwork extends WiredNetworkBase
{
    parameters:
        visualizer.typename = default("");
    submodules:
        sensor: StandardHost;
        secondSensor: StandardHost;
        gateway: StandardHost;
        alternative: StandardHost;
        broker: StandardHost;
        observer: StandardHost;
        switch: EthernetSwitch;
    connections:
        sensor.ethg++ <--> EthernetLink { datarate = 100Mbps; delay = 1ms; } <--> switch.ethg++;
        secondSensor.ethg++ <--> EthernetLink { datarate = 100Mbps; delay = 1ms; } <--> switch.ethg++;
        gateway.ethg++ <--> EthernetLink { datarate = 100Mbps; delay = 1ms; } <--> switch.ethg++;
        alternative.ethg++ <--> EthernetLink { datarate = 100Mbps; delay = 1ms; } <--> switch.ethg++;
        broker.ethg++ <--> EthernetLink { datarate = 100Mbps; delay = 1ms; } <--> switch.ethg++;
        observer.ethg++ <--> EthernetLink { datarate = 100Mbps; delay = 1ms; } <--> switch.ethg++;
}
