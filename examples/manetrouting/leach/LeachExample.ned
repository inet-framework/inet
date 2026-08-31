//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.examples.manetrouting.leach;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.contract.INetworkNode;
import inet.node.inet.SensorNode;
import inet.physicallayer.wireless.common.contract.packetlevel.IRadioMedium;
import inet.visualizer.contract.IIntegratedVisualizer;
import inet.environment.common.PhysicalEnvironment;

network LeachExample
{
    parameters:
        int numNodes;
        @display("bgb=200,200;bgg=100,1,grey95");
        @figure[title](type=label; pos=0,-1; anchor=sw; color=darkblue);
        
    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=100,100");
        }
        radioMedium: <default("UnitDiskRadioMedium")> like IRadioMedium {
            @display("p=100,100");
        }
        physicalEnvironment: PhysicalEnvironment {
            @display("p=100,100");
        }
        host[numNodes]: <default("WirelessHost")> like INetworkNode {
            @display("i=misc/sensor2");
        }
        baseStation: <default("WirelessHost")> like INetworkNode {
            @display("p=50,50;i=misc/sensorgateway");
        }
}
