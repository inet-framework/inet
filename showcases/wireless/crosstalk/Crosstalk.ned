package inet.showcases.wireless.crosstalk;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.AdhocHost;
import inet.physicallayer.wireless.ieee80211.packetlevel.Ieee80211RadioMedium;
import inet.visualizer.canvas.integrated.IntegratedCanvasVisualizer;


network CrosstalkShowcaseBaseNetwork
{
    @display("bgb=661,400");
    submodules:
        host1: AdhocHost {
            @display("p=231,201");
        }
        host2: AdhocHost {
            @display("p=459,201");
        }
        host3: AdhocHost {
            @display("p=346,94");
        }
        host4: AdhocHost {
            @display("p=346,306");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=75,55");
        }
        visualizer: IntegratedCanvasVisualizer {
            @display("p=75,125");
        }
}

network CrosstalkShowcaseOneRadioMediumModule extends CrosstalkShowcaseBaseNetwork
{
    submodules:
        radioMedium: Ieee80211RadioMedium {
            @display("p=74,191");
        }
}

network CrosstalkShowcaseTwoRadioMediumModules extends CrosstalkShowcaseBaseNetwork
{
    submodules:
        radioMedium1: Ieee80211RadioMedium {
            @display("p=74,191");
        }
        radioMedium2: Ieee80211RadioMedium {
            @display("p=74,255");
        }
}
