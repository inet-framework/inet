

package inet.showcases.wireless.sensornetwork;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ethernet.Eth100M;
import inet.node.inet.SensorNode;
import inet.node.inet.StandardHost;
import inet.physicallayer.wireless.apsk.packetlevel.ApskScalarRadioMedium;
import inet.routing.contract.IManetRouting;
import inet.visualizer.common.IntegratedVisualizer;


module ManetSensorNode extends SensorNode
{
    submodules:
        routing: <default("")> like IManetRouting if typename != "" {
            @display("p=913,225");
        }
    connections:
        routing.ipOut --> tn.in++ if exists(routing);
        routing.ipIn <-- tn.out++ if exists(routing);
}

network ManetSensorNetwork
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
        sensor4: ManetSensorNode {
            @display("p=17.290247,6.6757708;i=misc/sensor2");
        }
        sensor1: ManetSensorNode {
            @display("p=11.482326,21.696255;i=misc/sensor2");
        }
        sensor2: ManetSensorNode {
            @display("p=28.772572,30.775303;i=misc/sensor2");
        }
        sensor3: ManetSensorNode {
            @display("p=32.978306,6.6757708;i=misc/sensor2");
        }
        gateway: ManetSensorNode {
            @display("p=23.89926,17.557278;i=misc/sensorgateway");
        }
        server: StandardHost {
            @display("p=35.381584,17.557278");
        }
    connections:
        gateway.ethg++ <--> Eth100M <--> server.ethg++;
}
