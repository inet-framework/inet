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
import inet.node.inet.StandardHost;

network NetworkWithCrossTraffic
{
    @display("bgb=1134,360");
    submodules:
        gptpBridge0: GptpBridge {
            @display("p=230,179");
        }
        gptpMaster: GptpMaster {
            @display("p=91,180");
        }
        gptpBridge1: GptpBridge {
            @display("p=370,179");
        }
        gptpBridge2: GptpBridge {
            @display("p=510,179");
        }
        gptpBridge3: GptpBridge {
            @display("p=651,179");
        }
        gptpBridge4: GptpBridge {
            @display("p=794,179");
        }
        gptpBridge5: GptpBridge {
            @display("p=927,179");
        }
        gptpSlave: GptpSlave {
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
        configurator: Ipv4NetworkConfigurator {
            @display("p=83,38");
        }
    connections:
        gptpMaster.ethg++ <--> {  datarate = 100Mbps; delay = 10ms; } <--> gptpBridge0.ethg++;
        standardHost0.ethg++ <--> {  datarate = 100Mbps; delay = 10ms; } <--> gptpBridge0.ethg++;
        standardHost1.ethg++ <--> {  datarate = 100Mbps; delay = 10ms; } <--> gptpBridge0.ethg++;
        gptpBridge0.ethg++ <--> {  datarate = 100Mbps; delay = 10ms; } <--> gptpBridge1.ethg++;
        standardHost2.ethg++ <--> {  datarate = 100Mbps; delay = 10ms; } <--> gptpBridge1.ethg++;
        standardHost3.ethg++ <--> {  datarate = 100Mbps; delay = 10ms; } <--> gptpBridge1.ethg++;
        gptpBridge1.ethg++ <--> {  datarate = 100Mbps; delay = 10ms; } <--> gptpBridge2.ethg++;
        standardHost4.ethg++ <--> {  datarate = 100Mbps; delay = 10ms; } <--> gptpBridge2.ethg++;
        standardHost5.ethg++ <--> {  datarate = 100Mbps; delay = 10ms; } <--> gptpBridge2.ethg++;
        gptpBridge2.ethg++ <--> {  datarate = 100Mbps; delay = 10ms; } <--> gptpBridge3.ethg++;
        standardHost6.ethg++ <--> {  datarate = 100Mbps; delay = 10ms; } <--> gptpBridge3.ethg++;
        standardHost7.ethg++ <--> {  datarate = 100Mbps; delay = 10ms; } <--> gptpBridge3.ethg++;
        gptpBridge3.ethg++ <--> {  datarate = 100Mbps; delay = 10ms; } <--> gptpBridge4.ethg++;
        standardHost8.ethg++ <--> {  datarate = 100Mbps; delay = 10ms; } <--> gptpBridge4.ethg++;
        standardHost9.ethg++ <--> {  datarate = 100Mbps; delay = 10ms; } <--> gptpBridge4.ethg++;
        gptpBridge4.ethg++ <--> {  datarate = 100Mbps; delay = 10ms; } <--> gptpBridge5.ethg++;
        standardHost10.ethg++ <--> {  datarate = 100Mbps; delay = 10ms; } <--> gptpBridge5.ethg++;
        standardHost11.ethg++ <--> {  datarate = 100Mbps; delay = 10ms; } <--> gptpBridge5.ethg++;
        gptpBridge5.ethg++ <--> {  datarate = 100Mbps; delay = 10ms; } <--> gptpSlave.ethg++;
}
