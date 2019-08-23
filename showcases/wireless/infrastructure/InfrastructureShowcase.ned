package inet.showcases.wireless.infrastructure;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.WirelessHost;
import inet.node.wireless.AccessPoint;
import inet.physicallayer.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.visualizer.integrated.IntegratedVisualizer;


network InfrastructureShowcaseBase
{
    @display("bgb=678,294");
    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=52,45");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            @display("p=51,112");
        }
        visualizer: IntegratedVisualizer {
            @display("p=52,164");
        }
        host1: WirelessHost {
            @display("p=212,112");
        }
        host2: WirelessHost {
            @display("p=525,112");
        }
}

network InfrastructureShowcaseA extends InfrastructureShowcaseBase
{
    parameters:
        @figure[title](type=label; text="Infrastructure mode"; pos=525,25; anchor=sw; color=darkblue);
    submodules:
        accessPoint: AccessPoint {
            @display("p=374,200");
        }
}

network InfrastructureShowcaseB extends InfrastructureShowcaseBase
{
    parameters:
        @figure[title](type=label; text="Adhoc mode"; pos=525,25; anchor=sw; color=darkblue);
}
