//
// IoT Sensor-to-Cloud Gateway: 802.15.4 + Ethernet + IPv4 Routing
//
// Battery-powered 802.15.4 sensor nodes send data to a gateway node,
// which forwards it over Ethernet to a cloud server.
// Compare energy consumption under different MAC protocols (CSMA, BMac, XMac)
// and sensor data rates.
//

package inet.examples.wireless.sensorgateway_opus;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.SensorNode;
import inet.node.inet.StandardHost;
import inet.physicallayer.wireless.ieee802154.packetlevel.Ieee802154NarrowbandScalarRadioMedium;
import inet.visualizer.common.IntegratedVisualizer;

network SensorGatewayNetwork
{
    submodules:
        visualizer: IntegratedVisualizer {
            @display("p=50,50");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=50,150");
        }
        radioMedium: Ieee802154NarrowbandScalarRadioMedium {
            @display("p=50,250");
        }
        sensor1: SensorNode {
            @display("p=150,100");
        }
        sensor2: SensorNode {
            @display("p=450,100");
        }
        sensor3: SensorNode {
            @display("p=150,400");
        }
        sensor4: SensorNode {
            @display("p=450,400");
        }
        gateway: StandardHost {
            @display("p=300,250;i=device/accesspoint");
        }
        server: StandardHost {
            @display("p=550,250;i=device/server");
        }
    connections:
        gateway.ethg++ <--> { datarate = 100Mbps; delay = 1us; } <--> server.ethg++;
}
