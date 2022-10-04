package inet.showcases.wireless.sensornetwork;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ethernet.Eth100M;
import inet.node.inet.SensorNode;
import inet.node.inet.StandardHost;
import inet.physicallayer.wireless.apsk.packetlevel.ApskScalarRadioMedium;
import inet.visualizer.common.IntegratedVisualizer;


network SensorNetworkShowcaseA
{
    @display("bgb=60,30;bgi=showcases/warehouse,s");
    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=3.4662657,4.033473");
        }
        visualizer: IntegratedVisualizer {
            @display("p=3.4662657,9.264383");
        }
        radioMedium: ApskScalarRadioMedium {
            @display("p=3.4032426,14.0541315");
        }
        sensor4: SensorNode {
            @display("p=22,6.5;i=misc/sensor2");
        }
        sensor1: SensorNode {
            @display("p=16.482326,20.196255;i=misc/sensor2");
        }
        sensor2: SensorNode {
            @display("p=33.772572,16.275303;i=misc/sensor2");
        }
        sensor3: SensorNode {
            @display("p=35,6.5;i=misc/sensor2");
        }
        gateway: SensorNode {
            @display("p=28.5,13;i=misc/sensorgateway");
        }
        server: StandardHost {
            @display("p=38.35591,12.96182");
        }
    connections:
        gateway.ethg++ <--> Eth100M <--> server.ethg++;
}

network SensorNetworkShowcaseB
{
    @display("bgb=40,40");
    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=4.0722203,4.0722203");
        }
        visualizer: IntegratedVisualizer {
            @display("p=4.0722203,9.346079");
        }
        radioMedium: ApskScalarRadioMedium {
            @display("p=4.0054626,14.219392");
        }
        sensor4: SensorNode {
            @display("p=17.290247,6.6757708;i=misc/sensor2");
        }
        sensor1: SensorNode {
            @display("p=11.482326,21.696255;i=misc/sensor2");
        }
        sensor2: SensorNode {
            @display("p=28.772572,30.775303;i=misc/sensor2");
        }
        sensor3: SensorNode {
            @display("p=32.978306,6.6757708;i=misc/sensor2");
        }
        gateway: SensorNode {
            @display("p=23.89926,17.557278;i=misc/sensorgateway");
        }
        server: StandardHost {
            @display("p=35.381584,17.557278");
        }
    connections:
        gateway.ethg++ <--> Eth100M <--> server.ethg++;
}

