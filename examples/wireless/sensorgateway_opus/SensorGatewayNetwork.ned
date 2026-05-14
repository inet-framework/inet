package inet.examples.wireless.sensorgateway_opus;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ethernet.Eth100M;
import inet.node.inet.SensorNode;
import inet.node.inet.StandardHost;
import inet.physicallayer.wireless.ieee802154.packetlevel.Ieee802154NarrowbandScalarRadioMedium;
import inet.visualizer.contract.IIntegratedVisualizer;

network SensorGatewayNetwork
{
    submodules:
        visualizer: <default(firstAvailableOrEmpty("IntegratedCanvasVisualizer"))> like IIntegratedVisualizer if typename != "" {
            @display("p=50,50;is=s");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=50,150;is=s");
        }
        radioMedium: Ieee802154NarrowbandScalarRadioMedium {
            @display("p=50,250;is=s");
        }
        sensor[4]: SensorNode {
            @display("is=s");
        }
        gateway: StandardHost {
            @display("p=400,250;i=device/accesspoint");
        }
        server: StandardHost {
            @display("p=650,250;i=device/server");
        }
    connections:
        gateway.ethg++ <--> Eth100M <--> server.ethg++;
}
