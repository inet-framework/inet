package inet.showcases.wireless.directionalantennas;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.AdhocHost;
import inet.physicallayer.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.visualizer.integrated.IntegratedVisualizer;


network DirectionalAntennasShowcase
{
    @display("bgb=700,400");
    @statistic[receptionPower](source="receptionMinSignalPower(radioMedium.signalArrivalStarted)"; record=vector);
    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=62,43;is=s");
        }
        visualizer: IntegratedVisualizer {
            @display("p=62,121;is=s");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            @display("p=62,199;is=s");
        }
        source: AdhocHost {
            @display("p=400,200");
        }
        destination: AdhocHost {
            @display("p=400,350");
        }
}
