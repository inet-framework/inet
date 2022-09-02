package inet.showcases.mobility.basic;

import inet.node.inet.StandardHost;
import inet.visualizer.common.IntegratedVisualizer;

network BasicMobilityShowcase
{
    parameters:
        int numHosts;
        @display("bgb=400,400");
    submodules:
        host[numHosts]: StandardHost {
            @display("i=misc/smartphone");
        }
        visualizer: IntegratedVisualizer {
            @display("p=50,50");
        }
}