//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.examples.wireless.sensorgateway_kimi;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ethernet.Eth100M;
import inet.node.inet.SensorNode;
import inet.node.inet.StandardHost;
import inet.physicallayer.wireless.ieee802154.packetlevel.Ieee802154NarrowbandScalarRadioMedium;
import inet.visualizer.contract.IIntegratedVisualizer;

network SensorGatewayNetwork
{
    @display("bgb=500,350");
    submodules:
        visualizer: <default(firstAvailableOrEmpty("IntegratedCanvasVisualizer"))> like IIntegratedVisualizer if typename != "" {
            parameters:
                @display("p=50,50;is=s");
        }
        configurator: Ipv4NetworkConfigurator {
            parameters:
                @display("p=50,100;is=s");
        }
        radioMedium: Ieee802154NarrowbandScalarRadioMedium {
            parameters:
                @display("p=50,150;is=s");
        }
        sensor1: SensorNode {
            parameters:
                @display("p=140,120;i=misc/sensor2");
        }
        sensor2: SensorNode {
            parameters:
                @display("p=260,120;i=misc/sensor2");
        }
        sensor3: SensorNode {
            parameters:
                @display("p=140,210;i=misc/sensor2");
        }
        sensor4: SensorNode {
            parameters:
                @display("p=260,210;i=misc/sensor2");
        }
        gateway: StandardHost {
            parameters:
                @display("p=200,165;i=device/accesspoint");
                numWlanInterfaces = 1;
                numEthInterfaces = 1;
                forwarding = true;
        }
        server: StandardHost {
            parameters:
                @display("p=380,165;i=device/pc2");
                numEthInterfaces = 1;
        }
    connections:
        gateway.ethg++ <--> Eth100M <--> server.ethg++;
}
