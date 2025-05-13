package step2_frame_preemption;

import Wfcs2025NetworkBase;
import inet.node.ethernet.Eth100M;
import inet.node.tsn.TsnDevice;

network FramePreemptionShowcase extends Wfcs2025NetworkBase
{
    submodules:
        source: TsnDevice {
            @display("p=500,100");
        }
        destination: TsnDevice {
            @display("p=1100,100");
        }
    connections:
        source.ethg++ <--> Eth100M <--> destination.ethg++;
}

