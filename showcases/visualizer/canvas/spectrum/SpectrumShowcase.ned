package inet.showcases.visualizer.canvas.spectrum;

import inet.environment.common.PhysicalEnvironment;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.AdhocHost;
import inet.node.wireless.Probe;
import inet.physicallayer.wireless.ieee80211.packetlevel.Ieee80211DimensionalRadioMedium;
import inet.visualizer.common.IntegratedVisualizer;


network SpectrumShowcaseSpectrumFigureSpectrogramNetwork
{
    @display("bgb=661,400");
    submodules:
        host1: AdhocHost {
            @display("p=285,149");
        }
        host2: AdhocHost {
            @display("p=513,149");
        }
        host3: AdhocHost {
            @display("p=400,42");
        }
        host4: AdhocHost {
            @display("p=400,254");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=75,55");
        }
        visualizer: IntegratedVisualizer {
            @display("p=75,125");
        }
        radioMedium: Ieee80211DimensionalRadioMedium {
            @display("p=75,184");
        }
        physicalEnvironment: PhysicalEnvironment {
            @display("p=75,257");
        }
        probe: Probe {
            @display("p=180,42");
        }
}

network SpectrumShowcasePowerDensityMapNetwork
{
    @display("bgb=661,400");
    submodules:
        host1: AdhocHost {
            @display("p=228,119");
        }
        host2: AdhocHost {
            @display("p=316,81.5");
        }
        host3: AdhocHost {
            @display("p=285,120");
        }
        host4: AdhocHost {
            @display("p=251,150");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=75,55");
        }
        visualizer: IntegratedVisualizer {
            @display("p=75,125");
        }
        radioMedium: Ieee80211DimensionalRadioMedium {
            @display("p=74,184");
        }
        physicalEnvironment: PhysicalEnvironment {
            @display("p=65,257");
        }
}
