
package step5_gate_scheduling;

import Wfcs2025NetworkBase;
import inet.linklayer.configurator.gatescheduling.contract.IGateScheduleConfigurator;
import inet.node.ethernet.EthernetLink;
import inet.node.tsn.TsnDevice;
import inet.showcases.tsn.gatescheduling.TsnSwitch1;
import inet.showcases.tsn.gatescheduling.TsnSwitch2;

network GateSchedulingShowcase extends Wfcs2025NetworkBase
{
    submodules:
        gateScheduleConfigurator: <default("")> like IGateScheduleConfigurator if typename != "" {
            @display("p=200,400;is=s");
        }
        client1: TsnDevice {
            @display("p=500,150");
        }
        client2: TsnDevice {
            @display("p=500,350");
        }
        switch1: TsnSwitch1 {
            @display("p=800,250;i=device/switch");
        }
        switch2: TsnSwitch2 {
            @display("p=1100,250;i=device/switch");
        }
        server1: TsnDevice {
            @display("p=1400,150");
        }
        server2: TsnDevice {
            @display("p=1400,350");
        }
    connections:
        client1.ethg++ <--> EthernetLink <--> switch1.ethg++;
        client2.ethg++ <--> EthernetLink <--> switch1.ethg++;
        switch2.ethg++ <--> EthernetLink <--> server1.ethg++;
        switch2.ethg++ <--> EthernetLink <--> server2.ethg++;
        switch1.ethg++ <--> EthernetLink <--> switch2.ethg++;
}

