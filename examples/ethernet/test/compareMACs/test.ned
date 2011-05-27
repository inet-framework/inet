
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
        }
        host_iq_1_F_B: EtherHost {
            parameters:
                @display("p=210,70");
        }
        host_eq_1_F_A: EtherHost {
            parameters:
                @display("p=70,140");
                queueType = "DropTailQueue";
        }
        host_eq_1_F_B: EtherHost {
            parameters:
                @display("p=210,140");
                queueType = "DropTailQueue";
        }
        host_iq_2_F_A: EtherHost {
            parameters:
                @display("p=70,210");
                csmacdSupport = false;
        }
        host_iq_2_F_B: EtherHost {
            parameters:
                @display("p=210,210");
                csmacdSupport = false;
        }
        host_eq_2_F_A: EtherHost {
            parameters:
                @display("p=70,280");
                csmacdSupport = false;
                queueType = "DropTailQueue";
        }
        host_eq_2_F_B: EtherHost {
            parameters:
                @display("p=210,280");
                csmacdSupport = false;
                queueType = "DropTailQueue";
        }

    connections:
        host_iq_1_F_A.ethg <--> C <--> host_iq_1_F_B.ethg;
        host_eq_1_F_A.ethg <--> C <--> host_eq_1_F_B.ethg;
        host_iq_2_F_A.ethg <--> C <--> host_iq_2_F_B.ethg;
        host_eq_2_F_A.ethg <--> C <--> host_eq_2_F_B.ethg;
}

