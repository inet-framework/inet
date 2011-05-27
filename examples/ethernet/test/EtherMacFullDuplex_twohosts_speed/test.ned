
package inet.examples.ethernet.test.EtherMacFullDuplex_twohosts_speed;

import ned.DatarateChannel;
import inet.nodes.ethernet.EtherHost;


module EtherHostFullDuplexQ extends EtherHost
{
    parameters:
		csmacdSupport = false;
        queueType = "DropTailQueue";
}

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
        host_10_F_A: EtherHostFullDuplexQ {
            parameters:
                @display("p=70,140");
        }
        host_10_F_B: EtherHostFullDuplexQ {
            parameters:
                @display("p=210,140");
        }
        host_100_F_A: EtherHostFullDuplexQ {
            parameters:
                @display("p=70,280");
        }
        host_100_F_B: EtherHostFullDuplexQ {
            parameters:
                @display("p=210,280");
        }
        host_1000_F_A: EtherHostFullDuplexQ {
            parameters:
                @display("p=70,350");
        }
        host_1000_F_B: EtherHostFullDuplexQ {
            parameters:
                @display("p=210,350");
        }
    connections:
        host_10_F_A.ethg <--> C10 <--> host_10_F_B.ethg;
        host_100_F_A.ethg <--> C100 <--> host_100_F_B.ethg;
        host_1000_F_A.ethg <--> C1000 <--> host_1000_F_B.ethg;
}

