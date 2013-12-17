//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

package inet.examples.ieee8021d.RSTP;

import ned.DatarateChannel;
import inet.nodes.Ieee8021d.RSTPSwitch;
import inet.nodes.ethernet.EtherHost;

network CompleteNetwork
{
    @display("bgb=689,368");
    types:
        channel C extends DatarateChannel
        {
            delay = 0.1us;
            datarate = 100Mbps;
        }
    submodules:
        switchBEB1: RSTPSwitch {
            parameters:
                @display("p=333,78");

            gates:
                ethgB[2];

        }
        switchBEB2: RSTPSwitch {
            parameters:
                @display("p=301,217");

            gates:
                ethgB[3];
        }
        switchBEB3: RSTPSwitch {
            parameters:
                @display("p=443,78");

            gates:
                ethgB[2];

        }
        switchBEB4: RSTPSwitch {
            parameters:
                @display("p=322,153");

            gates:
                ethgB[4];

        }
        switchBEB5: RSTPSwitch {
            parameters:
                @display("p=529,217");

            gates:
                ethgB[1];

        }
        switchBEB6: RSTPSwitch {
            parameters:
                @display("p=148,217");

            gates:
                ethgB[2];

        }
        switchBEB7: RSTPSwitch {
            parameters:
                @display("p=443,153");

            gates:
                ethgB[4];

        }
    connections:
        switchBEB1.ethgB[0] <--> C <--> switchBEB4.ethgB[0];
        switchBEB1.ethgB[1] <--> C <--> switchBEB3.ethgB[0];
        switchBEB2.ethgB[0] <--> C <--> switchBEB4.ethgB[2];
        switchBEB2.ethgB[1] <--> C <--> switchBEB6.ethgB[1];
        switchBEB2.ethgB[2] <--> C <--> switchBEB7.ethgB[2];
        switchBEB3.ethgB[1] <--> C <--> switchBEB7.ethgB[0];
        switchBEB4.ethgB[1] <--> C <--> switchBEB6.ethgB[0];
        switchBEB4.ethgB[3] <--> C <--> switchBEB7.ethgB[1];
        switchBEB5.ethgB[0] <--> C <--> switchBEB7.ethgB[3];
}


network NetworkWithBK
{
    @display("bgb=689,368");
    types:
        channel C extends DatarateChannel
        {
            delay = 0.1us;
            datarate = 100Mbps;
        }
    submodules:
        switchBEB1: RSTPSwitch {
            parameters:
                @display("p=333,78");

            gates:
                ethgB[2];

        }
        switchBEB2: RSTPSwitch {
            parameters:
                @display("p=301,217");

            gates:
                ethgB[5];
        }
        switchBEB3: RSTPSwitch {
            parameters:
                @display("p=443,78");


            gates:
                ethgB[2];

        }
        switchBEB4: RSTPSwitch {
            parameters:
                @display("p=322,153");

            gates:
                ethgB[4];
        }
        switchBEB5: RSTPSwitch {
            parameters:
                @display("p=529,217");

            gates:
                ethgB[2];
        }
        switchBEB6: RSTPSwitch {
            parameters:
                @display("p=148,217");

            gates:
                ethgB[2];
        }
        switchBEB7: RSTPSwitch {
            parameters:
                @display("p=443,153");

            gates:
                ethgB[5];

        }
    connections:
        switchBEB1.ethgB[0] <--> C <--> switchBEB4.ethgB[0];
        switchBEB1.ethgB[1] <--> C <--> switchBEB3.ethgB[0];
        switchBEB2.ethgB[0] <--> C <--> switchBEB4.ethgB[2];
        switchBEB2.ethgB[1] <--> C <--> switchBEB6.ethgB[1];
        switchBEB2.ethgB[2] <--> C <--> switchBEB7.ethgB[2];
        switchBEB3.ethgB[1] <--> C <--> switchBEB7.ethgB[0];
        switchBEB4.ethgB[1] <--> C <--> switchBEB6.ethgB[0];
        switchBEB4.ethgB[3] <--> C <--> switchBEB7.ethgB[1];
        switchBEB5.ethgB[0] <--> C <--> switchBEB7.ethgB[3];
        switchBEB5.ethgB[1] <--> C <--> switchBEB7.ethgB[4];
        switchBEB2.ethgB[3] <--> C <--> switchBEB2.ethgB[4];
}


network NetworkWithHosts
{
    @display("bgb=689,368");
    types:
        channel C extends DatarateChannel
        {
            delay = 0.1us;
            datarate = 100Mbps;
        }
    submodules:
        switchBEB1: RSTPSwitch {
            parameters:
                @display("p=333,78");

            gates:
                ethgB[2];

        }
        switchBEB2: RSTPSwitch {
            parameters:
                @display("p=301,217");

            gates:
                ethgB[3];
        }
        switchBEB3: RSTPSwitch {
            parameters:
                @display("p=443,78");

            gates:
                ethgB[3];

        }
        switchBEB4: RSTPSwitch {
            parameters:
                @display("p=322,153");

            gates:
                ethgB[4];

        }
        switchBEB5: RSTPSwitch {
            parameters:
                @display("p=529,217");

            gates:
                ethgB[2];

        }
        switchBEB6: RSTPSwitch {
            parameters:
                @display("p=148,217");

            gates:
                ethgB[3];

        }
        switchBEB7: RSTPSwitch {
            parameters:
                @display("p=443,153");

            gates:
                ethgB[4];

        }
        host1: EtherHost {
            @display("p=77,217");
        }
        host2: EtherHost {
            @display("p=609,217");
        }
        host3: EtherHost {
            @display("p=529,78");
        }
    connections:
        switchBEB1.ethgB[0] <--> C <--> switchBEB4.ethgB[0];
        switchBEB1.ethgB[1] <--> C <--> switchBEB3.ethgB[0];
        switchBEB2.ethgB[0] <--> C <--> switchBEB4.ethgB[2];
        switchBEB2.ethgB[1] <--> C <--> switchBEB6.ethgB[1];
        switchBEB2.ethgB[2] <--> C <--> switchBEB7.ethgB[2];
        switchBEB3.ethgB[1] <--> C <--> switchBEB7.ethgB[0];
        switchBEB4.ethgB[1] <--> C <--> switchBEB6.ethgB[0];
        switchBEB4.ethgB[3] <--> C <--> switchBEB7.ethgB[1];
        switchBEB5.ethgB[0] <--> C <--> switchBEB7.ethgB[3];
        switchBEB3.ethgB[2] <--> C <--> host3.ethg;
        switchBEB5.ethgB[1] <--> C <--> host2.ethg;
        switchBEB6.ethgB[2] <--> C <--> host1.ethg;
}
