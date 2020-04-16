package inet.showcases.mobility.combining;

import inet.node.inet.StandardHost;
import inet.visualizer.integrated.IntegratedVisualizer;


import inet.physicallayer.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.visualizer.integrated.IntegratedMultiVisualizer;


network MobilityShowcase
{
    parameters:
        int numHosts;
        @display("bgb=400,400");
    submodules:
        host[numHosts]: StandardHost {
            @display("i=misc/smartphone");
        }
        visualizer: IntegratedVisualizer {
            @display("p=47,39");
        }
}


network AntennaOrientation
{
    @display("bgb=800,800;bgi=showcases/desert,s");
    string minus = default("-");

    submodules:
        rover: StandardHost {
            @display("p=247.632,395.568;b=100,64,rect,$minus,$minus;i=-");
        }
        visualizer: IntegratedMultiVisualizer {
            @display("p=61.104,57.888");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            @display("p=173.664,56.28");
        }
        drone: StandardHost {
            @display("p=43,214;i=misc/drone");
        }
        base: StandardHost {
            @display("p=582.096,201;i=device/antennatower");
        }
}