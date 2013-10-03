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

package inet.examples.ethernet.l2configurator;

import ned.DatarateChannel;
import inet.nodes.ethernet.EtherSwitch;
import inet.nodes.ethernet.EtherHost;
import inet.nodes.inet.StandardHost;
import inet.nodes.rstp.RSTPSwitch;
import inet.linklayer.configurator.L2NetworkConfigurator;

channel cable extends DatarateChannel
{
    parameters:
        delay = 0.1us;
        datarate = 100Mbps;
}

network Test
{
    @display("bgb=309,273");
    submodules:
        Switch1: RSTPSwitch {
            @display("p=157,102");
            gates:
                ethg[];
        }
        Switch2: RSTPSwitch {
            @display("p=96,175");
            gates:
                ethg[];
        }
        Switch3: RSTPSwitch {
            @display("p=205,175");
            gates:
                ethg[];
        }
        Host1: EtherHost {
            @display("p=157,36");
        }
        Host2: EtherHost {
            @display("p=35,214");
        }
        Host3: EtherHost {
            @display("p=266,214");
        }

        l2NetworkConfigurator: L2NetworkConfigurator {
            @display("p=56,36;is=s");
        }
    connections:
        Switch1.ethg++ <--> cable <--> Switch2.ethg++;
        Switch2.ethg++ <--> cable <--> Switch3.ethg++;
        Switch1.ethg++ <--> cable <--> Switch3.ethg++;

        Host1.ethg <--> cable <--> Switch1.ethg++;
        Host2.ethg <--> cable <--> Switch2.ethg++;
        Host3.ethg <--> cable <--> Switch3.ethg++;
}
