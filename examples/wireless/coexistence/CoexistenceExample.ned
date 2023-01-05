package inet.examples.wireless.coexistence;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.AdhocHost;
import inet.physicallayer.wireless.common.contract.packetlevel.IRadioMedium;
import inet.physicallayer.wireless.noise.NoiseSource;
import inet.visualizer.canvas.integrated.IntegratedCanvasVisualizer;


network UnitDiskScalarNetwork
{
    @display("bgb=400,400");
    submodules:

        configurator: Ipv4NetworkConfigurator {
            @display("p=35,40");
        }
        radioMedium: <> like IRadioMedium {
            @display("p=32,113");
        }
        visualizer: IntegratedCanvasVisualizer {
            @display("p=37,185");
        }
        wifiSource: AdhocHost {
            @display("p=144,79");
        }
        wifiSink: AdhocHost {
            @display("p=105,154");
        }
        wpanSource: AdhocHost {
            @display("p=350,79");
        }
        wpanSink: AdhocHost {
            @display("p=298,160");
        }
        apskSource: AdhocHost {
            @display("p=231,48");
        }
        apskSink: AdhocHost {
            @display("p=200,210");
        }
}

network DimensionalNetwork extends UnitDiskScalarNetwork
{
    submodules:
        noiseSource: NoiseSource {
            @display("p=221,132");
        }
        uwbSource: AdhocHost {
            @display("p=293,41");
        }
        uwbSink: AdhocHost {
            @display("p=251,217");
        }
}
