
package inet.examples.ethernet.test.burstTests;

import ned.DatarateChannel;
import inet.nodes.ethernet.EtherHost;
import inet.nodes.ethernet.EtherHostQ;

network BurstTests
{
    types:
        channel C extends DatarateChannel
        {
            delay = 0s;
            datarate = 1000Mbps;
        }

    submodules:
        host_1_H_A: EtherHostQ {
            parameters:
                @display("p=70,70");
        }
        host_1_H_B: EtherHostQ {
            parameters:
                @display("p=210,70");
        }
        host_1_F_A: EtherHostQ {
            parameters:
                @display("p=70,140");
        }
        host_1_F_B: EtherHostQ {
            parameters:
                @display("p=210,140");
        }
        host_2_H_A: EtherHostQ {
            parameters:
                @display("p=70,210");
        }
        host_2_H_B: EtherHost {
            parameters:
                @display("p=210,210");
        }
        host_2_F_A: EtherHostQ {
            parameters:
                macType = "EtherMACFullDuplex";
                @display("p=70,280");
        }
        host_2_F_B: EtherHostQ {
            parameters:
                macType = "EtherMACFullDuplex";
                @display("p=210,280");
        }

    connections:
        host_1_H_A.ethg <--> C <--> host_1_H_B.ethg;
        host_1_F_A.ethg <--> C <--> host_1_F_B.ethg;
        host_2_H_A.ethg <--> C <--> host_2_H_B.ethg;
        host_2_F_A.ethg <--> C <--> host_2_F_B.ethg;
}
