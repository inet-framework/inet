package inet.showcases.wireless.ieee802154;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ethernet.Eth100M;
import inet.node.inet.SensorNode;
import inet.node.inet.StandardHost;
import inet.node.inet.WirelessHost;
import inet.physicallayer.apskradio.packetlevel.ApskScalarRadioMedium;
import inet.physicallayer.ieee802154.bitlevel.Ieee802154UwbIrRadioMedium;
import inet.physicallayer.ieee802154.packetlevel.Ieee802154NarrowbandScalarRadioMedium;
import inet.visualizer.integrated.IntegratedMultiVisualizer;
import inet.visualizer.integrated.IntegratedVisualizer;


network Ieee802154Showcase
{
    @display("bgb=20,20;bgi=showcases/floorplan,s");
    submodules:
        sensor1: SensorNode {
            @display("p=9.5,3.5;i=misc/sensor");
        }
        sensor2: SensorNode {
            @display("p=9.5,16.5;i=misc/sensor");
        }
        sensor3: SensorNode {
            @display("p=10.75,10.75;i=misc/sensor");
        }
        sensor4: SensorNode {
            @display("p=2.5,17.5;i=misc/sensor");
        }
        sensor5: SensorNode {
            @display("p=2.5,7;i=misc/sensor");
        }
        lamp1: SensorNode {
            @display("p=8,5;i=status/bulb");
        }
        lamp2: SensorNode {
            @display("p=8.75,13.5;i=status/bulb");
        }
        lamp3: SensorNode {
            @display("p=13.5,8.5;i=status/bulb");
        }
        lamp4: SensorNode {
            @display("p=16.5,9.5;i=status/bulb");
        }
        lamp5: SensorNode {
            @display("p=13.5,14;i=status/bulb");
        }
        lamp6: SensorNode {
            @display("p=4,13;i=status/bulb");
        }
        lamp7: SensorNode {
            @display("p=4,10.5;i=status/bulb");
        }
        lamp8: SensorNode {
            @display("p=4,5;i=status/bulb");
        }
        controller: SensorNode {
            @display("p=6.5,10.5;i=misc/sensorgateway");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=18,0.75");
        }
        radioMedium: Ieee802154NarrowbandScalarRadioMedium {
            @display("p=18,2.25");
        }
        visualizer: IntegratedMultiVisualizer {
            @display("p=18,3.75");
        }
}
