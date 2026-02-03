package inet.tutorials.ospf;

import inet.node.ethernet.Eth100M;
import inet.node.ethernet.EthernetSwitch;
import inet.node.inet.StandardHost;
import inet.visualizer.canvas.integrated.IntegratedCanvasVisualizer;


module OspfLan
{
    parameters:
        int h; // number of hosts on the hub
        @display("i=old/cloud");
    gates:
        inout ethg[];
    submodules:
        switch: EthernetSwitch {
            @display("p=100,100");
        }
        host[h]: StandardHost {
            @display("p=199,159");
        }
        visualizer: IntegratedCanvasVisualizer {
            @display("p=750.1,49.399998");
        }
    connections:
        for i=0..sizeof(ethg)-1 {
            switch.ethg++ <--> ethg[i];
        }
        for i=0..h-1 {
            switch.ethg++ <--> Eth100M <--> host[i].ethg++;
        }
}

