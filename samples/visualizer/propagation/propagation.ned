



package inet.samples.visualizer.propagation;

import inet.environment.common.PhysicalEnvironment;
import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;
import inet.node.inet.AdhocHost;
import inet.node.inet.WirelessHost;
import inet.node.wireless.AccessPoint;
import inet.physicallayer.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.visualizer.integrated.IntegratedVisualizer;


network PropagationA
{
    @display("bgb=920,566");
    submodules:
        host2: WirelessHost {
            @display("p=645.645,270.27");
        }
        configurator: IPv4NetworkConfigurator {
            @display("p=853.545,46.2");
        }
        visualizer: IntegratedVisualizer {
            @display("p=853.545,123.585");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            @display("p=852.39,204.435");
        }
        host1: WirelessHost {
            @display("p=179,271");
        }
}

network PropagationB
{
    parameters:
        @display("bgb=1000,500");
    submodules:
        visualizer: IntegratedVisualizer {
            parameters:
                @display("p=100,100");
        }
        configurator: IPv4NetworkConfigurator {
            parameters:
                @display("p=100,200");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            parameters:
                @display("p=100,300");
        }
        source: AdhocHost {
            parameters:
                @display("p=300,200");
        }
        relay: AdhocHost {
            parameters:
                @display("p=600,300");
        }
        destination: AdhocHost {
            parameters:
                @display("p=900,200");
        }
}

network PropagationC
{
    parameters:
        @display("bgb=1000,500");
    submodules:
        visualizer: IntegratedVisualizer {
            parameters:
                @display("p=100,100");
        }
        configurator: IPv4NetworkConfigurator {
            parameters:
                @display("p=100,200");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            parameters:
                @display("p=100,300");
        }
        source1: AdhocHost {
            parameters:
                @display("p=300,200");
        }
        destination: AdhocHost {
            parameters:
                @display("p=601.14496,387.79498");
        }
        source2: AdhocHost {
            parameters:
                @display("p=900,200");
        }
        physicalEnvironment: PhysicalEnvironment {
            @display("p=100.399994,387.79498");
        }
}

network PropagationD
{
    @display("bgb=300000,300000");
    submodules:
        wirelessHost: WirelessHost {
            @display("p=118801.586,166802.22");
        }
        accessPoint: AccessPoint {
            @display("p=118801.39,166802.22");
        }
        configurator: IPv4NetworkConfigurator {
            @display("p=234003.11,38400.51");
        }
        visualizer: IntegratedVisualizer {
            @display("p=234603.12,75001");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            @display("p=229203.05,115801.54");
        }
        wirelessHost1: WirelessHost {
            @display("p=5400.072,6000.08");
        }
}
