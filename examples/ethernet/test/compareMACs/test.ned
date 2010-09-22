
package inet.examples.ethernet.test.compareMACs;

import ned.DatarateChannel;
import inet.nodes.ethernet.EtherHost;
import inet.nodes.ethernet.EtherHostQ;

network CompareMACs
{
    types:
        channel C extends DatarateChannel
        {
            delay = 0s;
            datarate = 1000Mbps;
        }

    submodules:
        host_iq_1_F_A: EtherHost {
            parameters:
                @display("p=70,70");
        }
        host_iq_1_F_B: EtherHost {
            parameters:
                @display("p=210,70");
        }
        host_eq_1_F_A: EtherHostQ {
            parameters:
                @display("p=70,140");
        }
        host_eq_1_F_B: EtherHostQ {
            parameters:
                @display("p=210,140");
        }
        host_iq_2_F_A: EtherHost {
            parameters:
                macType = "EtherMACFullDuplex";
                @display("p=70,210");
        }
        host_iq_2_F_B: EtherHost {
            parameters:
                macType = "EtherMACFullDuplex";
                @display("p=210,210");
        }
        host_eq_2_F_A: EtherHostQ {
            parameters:
                macType = "EtherMACFullDuplex";
                @display("p=70,280");
        }
        host_eq_2_F_B: EtherHostQ {
            parameters:
                macType = "EtherMACFullDuplex";
                @display("p=210,280");
        }

    connections:
        host_iq_1_F_A.ethg <--> C <--> host_iq_1_F_B.ethg;
        host_eq_1_F_A.ethg <--> C <--> host_eq_1_F_B.ethg;
        host_iq_2_F_A.ethg <--> C <--> host_iq_2_F_B.ethg;
        host_eq_2_F_A.ethg <--> C <--> host_eq_2_F_B.ethg;
}

