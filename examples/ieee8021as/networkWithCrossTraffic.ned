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
import inet.node.dymo.DYMORouter;
import inet.node.inet.StandardHost;


network NetworkWithCrossTraffic
{
    @display("bgb=1134,360");
    submodules:
        gPtpBridge0: GPtpBridge {
            @display("p=230,179");
        }
        gPtpMaster: GPtpMaster {
            @display("p=91,180");
        }
        gPtpBridge1: GPtpBridge {
            @display("p=370,179");
        }
        gPtpBridge2: GPtpBridge {
            @display("p=510,179");
        }
        gPtpBridge3: GPtpBridge {
            @display("p=651,179");
        }
        gPtpBridge4: GPtpBridge {
            @display("p=794,179");
        }
        gPtpBridge5: GPtpBridge {
            @display("p=927,179");
        }
        gPtpSlave: GPtpSlave {
            @display("p=1087,180");
        }
        standardHost0: StandardHost {
            @display("p=230,61");
        }
        standardHost1: StandardHost {
            @display("p=230,305");
        }
        standardHost2: StandardHost {
            @display("p=370,61");
        }
        standardHost3: StandardHost {
            @display("p=370,305");
        }
        standardHost4: StandardHost {
            @display("p=510,61");
        }
        standardHost5: StandardHost {
            @display("p=510,305");
        }
        standardHost6: StandardHost {
            @display("p=651,61");
        }
        standardHost7: StandardHost {
            @display("p=651,305");
        }
        standardHost8: StandardHost {
            @display("p=794,61");
        }
        standardHost9: StandardHost {
            @display("p=794,305");
        }
        standardHost10: StandardHost {
            @display("p=927,61");
        }
        standardHost11: StandardHost {
            @display("p=927,305");
        }
        configurator: IPv4NetworkConfigurator {
            @display("p=83,38");
        }
    connections:
        gPtpMaster.ethg++ <--> {  datarate = 100Mbps; delay = 10ms; } <--> gPtpBridge0.ethg++;
        standardHost0.ethg++ <--> {  datarate = 100Mbps; delay = 10ms; } <--> gPtpBridge0.ethg++;
        standardHost1.ethg++ <--> {  datarate = 100Mbps; delay = 10ms; } <--> gPtpBridge0.ethg++;
        gPtpBridge0.ethg++ <--> {  datarate = 100Mbps; delay = 10ms; } <--> gPtpBridge1.ethg++;
        standardHost2.ethg++ <--> {  datarate = 100Mbps; delay = 10ms; } <--> gPtpBridge1.ethg++;
        standardHost3.ethg++ <--> {  datarate = 100Mbps; delay = 10ms; } <--> gPtpBridge1.ethg++;
        gPtpBridge1.ethg++ <--> {  datarate = 100Mbps; delay = 10ms; } <--> gPtpBridge2.ethg++;
        standardHost4.ethg++ <--> {  datarate = 100Mbps; delay = 10ms; } <--> gPtpBridge2.ethg++;
        standardHost5.ethg++ <--> {  datarate = 100Mbps; delay = 10ms; } <--> gPtpBridge2.ethg++;
        gPtpBridge2.ethg++ <--> {  datarate = 100Mbps; delay = 10ms; } <--> gPtpBridge3.ethg++;
        standardHost6.ethg++ <--> {  datarate = 100Mbps; delay = 10ms; } <--> gPtpBridge3.ethg++;
        standardHost7.ethg++ <--> {  datarate = 100Mbps; delay = 10ms; } <--> gPtpBridge3.ethg++;
        gPtpBridge3.ethg++ <--> {  datarate = 100Mbps; delay = 10ms; } <--> gPtpBridge4.ethg++;
        standardHost8.ethg++ <--> {  datarate = 100Mbps; delay = 10ms; } <--> gPtpBridge4.ethg++;
        standardHost9.ethg++ <--> {  datarate = 100Mbps; delay = 10ms; } <--> gPtpBridge4.ethg++;
        gPtpBridge4.ethg++ <--> {  datarate = 100Mbps; delay = 10ms; } <--> gPtpBridge5.ethg++;
        standardHost10.ethg++ <--> {  datarate = 100Mbps; delay = 10ms; } <--> gPtpBridge5.ethg++;
        standardHost11.ethg++ <--> {  datarate = 100Mbps; delay = 10ms; } <--> gPtpBridge5.ethg++;
        gPtpBridge5.ethg++ <--> {  datarate = 100Mbps; delay = 10ms; } <--> gPtpSlave.ethg++;
}
