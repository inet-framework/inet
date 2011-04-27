
package inet.examples.ethernet.test.switch;

import ned.DatarateChannel;
import inet.nodes.ethernet.EtherSwitch;
import inet.nodes.ethernet.EtherHost;


//
// Sample Ethernet LAN: hosts connected via switch.
//
network SpeedTest
{
    types:
        channel C10 extends DatarateChannel
        {
            delay = 0s;
            datarate = 10Mbps;
        }
        channel C100 extends DatarateChannel
        {
            delay = 0s;
            datarate = 100Mbps;
        }
        channel C1000 extends DatarateChannel
        {
            delay = 0s;
            datarate = 1000Mbps;
        }
    submodules:
        host_10_F_A: EtherHost {
            parameters:
                @display("p=70,70");
        }
        host_10_F_B: EtherHost {
            parameters:
                @display("p=210,70");
        }
        host_100_F_A: EtherHost {
            parameters:
                @display("p=70,140");
        }
        host_100_F_B: EtherHost {
            parameters:
                @display("p=210,140");
        }
        host_1000_F_A: EtherHost {
            parameters:
                @display("p=70,210");
        }
        host_1000_F_B: EtherHost {
            parameters:
                @display("p=210,210");
        }
        switch: EtherSwitch {
            parameters:
                @display("p=140,140");
            gates:
                ethg[6];
        }
    connections:
        switch.ethg[0] <--> C10 <--> host_10_F_A.ethg;
        switch.ethg[1] <--> C10 <--> host_10_F_B.ethg;
        switch.ethg[2] <--> C100 <--> host_100_F_A.ethg;
        switch.ethg[3] <--> C100 <--> host_100_F_B.ethg;
        switch.ethg[4] <--> C1000 <--> host_1000_F_A.ethg;
        switch.ethg[5] <--> C1000 <--> host_1000_F_B.ethg;
}
