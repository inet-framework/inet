//
// @authors: Enkhtuvshin Janchivnyambuu
//           Henning Puttnies
//           Peter Danielis
//           University of Rostock, Germany
//  

package ieee8021as.simulations;

import ieee8021as.GPtpBridge;
import ieee8021as.GPtpMaster;
import ieee8021as.GPtpSlave;
import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;

network NetworkDaisyChain
{
    @display("bgb=534,441");
    types:
        channel Ethernet100 extends ned.DatarateChannel
        {
            datarate = 100Mbps;
            delay = 25ns;		// Length of connection is 5 meters
        }
    submodules:
        gPtpBridge0: GPtpBridge {
            @display("p=264,162");
        }
        gPtpBridge1: GPtpBridge {
            @display("p=264,258");
        }
        gPtpBridge2: GPtpBridge {
            @display("p=264,349");
        }
        gPtpMaster: GPtpMaster {
            @display("p=264,55");
        }
        gPtpSlave0: GPtpSlave {
            @display("p=408,94");
        }
        gPtpSlave1: GPtpSlave {
            @display("p=408,182");
        }
        gPtpSlave2: GPtpSlave {
            @display("p=408,259");
        }
        gPtpSlave3: GPtpSlave {
            @display("p=408,375");
        }
        gPtpSlave4: GPtpSlave {
            @display("p=124,375");
        }
        gPtpSlave5: GPtpSlave {
            @display("p=124,298");
        }
        gPtpSlave6: GPtpSlave {
            @display("p=124,205");
        }
        gPtpSlave7: GPtpSlave {
            @display("p=124,94");
        }
        configurator: IPv4NetworkConfigurator {
            @display("p=51,33");
        }
    connections:
       
        gPtpMaster.ethg++ <--> Ethernet100 <--> gPtpBridge0.ethg++;
        gPtpBridge0.ethg++ <--> Ethernet100 <--> gPtpSlave0.ethg++;
        gPtpBridge0.ethg++ <--> Ethernet100 <--> gPtpSlave1.ethg++;
        gPtpBridge0.ethg++ <--> Ethernet100 <--> gPtpSlave7.ethg++;
        gPtpBridge0.ethg++ <--> Ethernet100 <--> gPtpBridge1.ethg++;
        gPtpBridge1.ethg++ <--> Ethernet100 <--> gPtpSlave2.ethg++;
        gPtpBridge1.ethg++ <--> Ethernet100 <--> gPtpSlave6.ethg++;
        gPtpBridge1.ethg++ <--> Ethernet100 <--> gPtpSlave5.ethg++;
        gPtpBridge1.ethg++ <--> Ethernet100 <--> gPtpBridge2.ethg++;
        gPtpBridge2.ethg++ <--> Ethernet100 <--> gPtpSlave3.ethg++;
        gPtpBridge2.ethg++ <--> Ethernet100 <--> gPtpSlave4.ethg++;
}
