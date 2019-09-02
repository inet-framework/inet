package inet.showcases.emulation.babel;

import inet.node.inet.AdhocHost;
import inet.physicallayer.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.visualizer.contract.IIntegratedVisualizer;

network BabelShowcase
{
    parameters:
        int numHosts;
        @display("bgb=600,400");
    submodules:
        visualizer: <default("IntegratedMultiCanvasVisualizer")> like IIntegratedVisualizer if hasVisualizer() {
            parameters:
                @display("p=176,350;is=s");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            parameters:
                @display("p=62,352;is=s");
        }
        host[numHosts]: AdhocHost {
            parameters:
                @display("r=,,#707070;p=300,200");
        }
}

