package inet.showcases.mobility.basic;

import inet.node.inet.StandardHost;
import inet.visualizer.canvas.integrated.IntegratedCanvasVisualizer;

network BasicMobilityShowcase
{
    parameters:
        int numHosts;
        @display("bgb=400,400");
    submodules:
        host[numHosts]: StandardHost {
            @display("i=misc/smartphone");
        }
        visualizer: IntegratedCanvasVisualizer {
            @display("p=50,50");
        }
}