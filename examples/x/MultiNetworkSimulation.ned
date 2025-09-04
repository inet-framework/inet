//
// Multi-Network Simulation with 5 WiFi networks and L-MAC TDMA backbone
// Flat network structure using separate module vectors for each WiFi network
//

package inet.examples.x;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.ManetRouter;
import inet.physicallayer.wireless.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.physicallayer.wireless.apsk.packetlevel.ApskScalarRadioMedium;
import inet.visualizer.contract.IIntegratedVisualizer;
import inet.common.scenario.ScenarioManager;
import inet.networklayer.ipv4.RoutingTableRecorder;
import inet.environment.common.PhysicalEnvironment;

network MultiNetworkSimulation
{
    parameters:
        int nodesPerNetwork = default(20);
        @display("bgb=2000,2000");
        
    submodules:
        visualizer: <default("IntegratedCanvasVisualizer")> like IIntegratedVisualizer {
            parameters:
                @display("p=100,100;is=s");
        }
        
        configurator: Ipv4NetworkConfigurator {
            parameters:
                @display("p=100,200;is=s");
        }
        
        routingTableRecorder: RoutingTableRecorder {
            parameters:
                @display("p=100,300;is=s");
        }
        
        scenarioManager: ScenarioManager {
            parameters:
                script = default(xml("<scenario/>"));
                @display("p=100,400;is=s");
        }
        
        // Separate radio mediums for each WiFi network (IEEE 802.11)
        wifiRadioMedium1: Ieee80211ScalarRadioMedium {
            parameters:
                @display("p=250,250;is=s");
        }
        
        wifiRadioMedium2: Ieee80211ScalarRadioMedium {
            parameters:
                @display("p=850,250;is=s");
        }
        
        wifiRadioMedium3: Ieee80211ScalarRadioMedium {
            parameters:
                @display("p=1450,250;is=s");
        }
        
        wifiRadioMedium4: Ieee80211ScalarRadioMedium {
            parameters:
                @display("p=550,850;is=s");
        }
        
        wifiRadioMedium5: Ieee80211ScalarRadioMedium {
            parameters:
                @display("p=1150,850;is=s");
        }
        
        // Radio medium for L-MAC TDMA backbone network
        lmacRadioMedium: ApskScalarRadioMedium {
            parameters:
                @display("p=100,600;is=s");
        }
        
        // Physical environment for two-ray ground reflection model
        physicalEnvironment: PhysicalEnvironment {
            parameters:
                @display("p=100,700;is=s");
        }
        
        // WiFi Network 1 - Channel 1 (2.412 GHz) - 20 nodes - RED
        // wifiNet1Host[0] acts as gateway with dual interfaces
        wifiNet1Host[nodesPerNetwork]: ManetRouter {
            parameters:
                numWlanInterfaces = (index == 0 ? 2 : 1);
                @display("i=device/pocketpc_s,red;r=,,#FF0000");
        }
        
        // WiFi Network 2 - Channel 6 (2.437 GHz) - 20 nodes - BLUE
        // wifiNet2Host[0] acts as gateway with dual interfaces
        wifiNet2Host[nodesPerNetwork]: ManetRouter {
            parameters:
                numWlanInterfaces = (index == 0 ? 2 : 1);
                @display("i=device/pocketpc_s,blue;r=,,#0000FF");
        }
        
        // WiFi Network 3 - Channel 11 (2.462 GHz) - 20 nodes - GREEN
        // wifiNet3Host[0] acts as gateway with dual interfaces
        wifiNet3Host[nodesPerNetwork]: ManetRouter {
            parameters:
                numWlanInterfaces = (index == 0 ? 2 : 1);
                @display("i=device/pocketpc_s,green;r=,,#00FF00");
        }
        
        // WiFi Network 4 - Channel 2 (2.417 GHz) - 20 nodes - ORANGE
        // wifiNet4Host[0] acts as gateway with dual interfaces
        wifiNet4Host[nodesPerNetwork]: ManetRouter {
            parameters:
                numWlanInterfaces = (index == 0 ? 2 : 1);
                @display("i=device/pocketpc_s,orange;r=,,#FFA500");
        }
        
        // WiFi Network 5 - Channel 7 (2.442 GHz) - 20 nodes - PURPLE
        // wifiNet5Host[0] acts as gateway with dual interfaces
        wifiNet5Host[nodesPerNetwork]: ManetRouter {
            parameters:
                numWlanInterfaces = (index == 0 ? 2 : 1);
                @display("i=device/pocketpc_s,purple;r=,,#800080");
        }
        
        
    connections allowunconnected:
}
