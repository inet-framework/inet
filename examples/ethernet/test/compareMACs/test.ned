
package inet.examples.ethernet.test.compareMACs;

import ned.DatarateChannel;
import inet.nodes.ethernet.EtherHost;


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
                queueType = "NoQueue";
        }
        host_iq_1_F_B: EtherHost {
            parameters:
                @display("p=210,70");
                queueType = "NoQueue";
        }
        host_eq_1_F_A: EtherHost {
            parameters:
                @display("p=70,140");
        }
        host_eq_1_F_B: EtherHost {
            parameters:
                @display("p=210,140");
        }
        host_iq_2_F_A: EtherHost {
            parameters:
                @display("p=70,210");
                queueType = "NoQueue";
                csmacdSupported = false;
        }
        host_iq_2_F_B: EtherHost {
            parameters:
                @display("p=210,210");
                queueType = "NoQueue";
                csmacdSupported = false;
        }
        host_eq_2_F_A: EtherHost {
            parameters:
                @display("p=70,280");
                csmacdSupported = false;
        }
        host_eq_2_F_B: EtherHost {
            parameters:
                @display("p=210,280");
                csmacdSupported = false;
        }

    connections:
        host_iq_1_F_A.ethg <--> C <--> host_iq_1_F_B.ethg;
        host_eq_1_F_A.ethg <--> C <--> host_eq_1_F_B.ethg;
        host_iq_2_F_A.ethg <--> C <--> host_iq_2_F_B.ethg;
        host_eq_2_F_A.ethg <--> C <--> host_eq_2_F_B.ethg;
}

