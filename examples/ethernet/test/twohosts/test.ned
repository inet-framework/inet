
package inet.examples.ethernet.test.twohosts;

import ned.DatarateChannel;
import inet.nodes.ethernet.EtherHost;


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
        host_10_H_A: EtherHost {
            parameters:
                mac.duplexEnabled = false;
                @display("p=70,70");
        }
        host_10_H_B: EtherHost {
            parameters:
                mac.duplexEnabled = false;
                @display("p=210,70");
        }
        host_10_F_A: EtherHost {
            parameters:
                @display("p=70,140");
        }
        host_10_F_B: EtherHost {
            parameters:
                @display("p=210,140");
        }
        host_100_H_A: EtherHost {
            parameters:
                mac.duplexEnabled = false;
                @display("p=70,210");
        }
        host_100_H_B: EtherHost {
            parameters:
                mac.duplexEnabled = false;
                @display("p=210,210");
        }
        host_100_F_A: EtherHost {
            parameters:
                @display("p=70,280");
        }
        host_100_F_B: EtherHost {
            parameters:
                @display("p=210,280");
        }
        host_1000_F_A: EtherHost {
            parameters:
                @display("p=70,350");
        }
        host_1000_F_B: EtherHost {
            parameters:
                @display("p=210,350");
        }
    connections:
        host_10_H_A.ethg <--> C10 <--> host_10_H_B.ethg;
        host_10_F_A.ethg <--> C10 <--> host_10_F_B.ethg;
        host_100_H_A.ethg <--> C100 <--> host_100_H_B.ethg;
        host_100_F_A.ethg <--> C100 <--> host_100_F_B.ethg;
        host_1000_F_A.ethg <--> C1000 <--> host_1000_F_B.ethg;
}
