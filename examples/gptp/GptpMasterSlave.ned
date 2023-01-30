package inet.examples.gptp;

import inet.linklayer.ieee8021as.GptpMaster;
import inet.linklayer.ieee8021as.GptpSlave;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;

network GptpMasterSlave
{
    parameters:
        double delayTime @unit(s) = default(10ms);
        @display("bgb=600,400");
    submodules:
        gptpMaster: GptpMaster {
            @display("p=100,300");
        }
        gptpSlave: GptpSlave {
            @display("p=500,300");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=100,100");
        }
    connections:
        gptpMaster.ethg++ <--> {  datarate = 100Mbps; delay = parent.delayTime; } <--> gptpSlave.ethg++;
}
