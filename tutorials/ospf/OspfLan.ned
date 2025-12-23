package inet.tutorials.ospf;

import inet.visualizer.integrated.IntegratedCanvasVisualizer;
import inet.node.ethernet.Eth100M;
import inet.node.ethernet.EtherSwitch;
import inet.node.inet.StandardHost;

module OspfLan
{
    parameters:
        int h; // number of hosts on the hub
        @display("i=old/cloud");
    gates:
        inout ethg[];
    submodules:
        switch: EtherSwitch;
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

