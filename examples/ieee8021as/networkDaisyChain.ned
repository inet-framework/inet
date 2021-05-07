//
// @authors: Enkhtuvshin Janchivnyambuu
//           Henning Puttnies
//           Peter Danielis
//           University of Rostock, Germany
//

package inet.examples.ieee8021as;

import inet.linklayer.ieee8021as.GptpBridge;
import inet.linklayer.ieee8021as.GptpMaster;
import inet.linklayer.ieee8021as.GptpSlave;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;

network NetworkDaisyChain
{
    @display("bgb=534,441");
    types:
        channel Ethernet100 extends ned.DatarateChannel
        {
            datarate = 100Mbps;
            delay = 25ns;       // Length of connection is 5 meters
        }
    submodules:
        gptpBridge0: GptpBridge {
            @display("p=264,162");
        }
        gptpBridge1: GptpBridge {
            @display("p=264,258");
        }
        gptpBridge2: GptpBridge {
            @display("p=264,349");
        }
        gptpMaster: GptpMaster {
            @display("p=264,55");
        }
        gptpSlave0: GptpSlave {
            @display("p=408,94");
        }
        gptpSlave1: GptpSlave {
            @display("p=408,182");
        }
        gptpSlave2: GptpSlave {
            @display("p=408,259");
        }
        gptpSlave3: GptpSlave {
            @display("p=408,375");
        }
        gptpSlave4: GptpSlave {
            @display("p=124,375");
        }
        gptpSlave5: GptpSlave {
            @display("p=124,298");
        }
        gptpSlave6: GptpSlave {
            @display("p=124,205");
        }
        gptpSlave7: GptpSlave {
            @display("p=124,94");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=51,33");
        }
    connections:

        gptpMaster.ethg++ <--> Ethernet100 <--> gptpBridge0.ethg++;
        gptpBridge0.ethg++ <--> Ethernet100 <--> gptpSlave0.ethg++;
        gptpBridge0.ethg++ <--> Ethernet100 <--> gptpSlave1.ethg++;
        gptpBridge0.ethg++ <--> Ethernet100 <--> gptpSlave7.ethg++;
        gptpBridge0.ethg++ <--> Ethernet100 <--> gptpBridge1.ethg++;
        gptpBridge1.ethg++ <--> Ethernet100 <--> gptpSlave2.ethg++;
        gptpBridge1.ethg++ <--> Ethernet100 <--> gptpSlave6.ethg++;
        gptpBridge1.ethg++ <--> Ethernet100 <--> gptpSlave5.ethg++;
        gptpBridge1.ethg++ <--> Ethernet100 <--> gptpBridge2.ethg++;
        gptpBridge2.ethg++ <--> Ethernet100 <--> gptpSlave3.ethg++;
        gptpBridge2.ethg++ <--> Ethernet100 <--> gptpSlave4.ethg++;
}
