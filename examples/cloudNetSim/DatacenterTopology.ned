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

package inet.examples.cloudNetSim;
import inet.node.inet.Router;
import inet.greencloudsimulator.myStandardHost;

import inet.node.inet.StandardHost;
import inet.node.ethernet.EtherHost;
import inet.node.ethernet.EtherSwitch;
import ned.DatarateChannel;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.networklayer.configurator.ipv4.Ipv4FlatNetworkConfigurator;

import inet.greencloudsimulator.RouterGreenCloud;
import inet.node.ethernet.Eth100M;

module Racking
{
    parameters:
        int N @prompt("Nodes per rack") = default(1);
        @display("bgb=506,467");
    gates:
        inout iogate[];
    submodules:
        computingServer[N]: myStandardHost {
            @display("i=device/server2;p=64,298,m,8,80,80");
        }
        accessRouter: RouterGreenCloud {
            @display("p=202,148");
        }

        
    connections allowunconnected:
        for i=0..N-1 {
            accessRouter.ethg++ <--> Eth100M <--> computingServer[i].ethg++;
        }
        accessRouter.ethg++ <--> iogate++;
        //accessRouter.ethg++ <--> iogate++;// based on racks input from datacenter
        //accessRouter.ethg++ <--> iogate++;
}

network SimpleDatacenterTopology
{
    parameters:
        int servercount = default(2);//@prompt("Number of Racks ") = default(2);
        int aggregateswitch = default(8);
        int corerouter = default(8);
        int accessswitch = default(16);
        //int ins @prompt("Number of TaskGenerator");
        @display("bgb=954,456");
    
    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=887,17");
        }

        AS1[aggregateswitch]: RouterGreenCloud {
            @display("i=abstract/router2;p=138,220,m,16,80,80");
        }

        CS1[corerouter]: RouterGreenCloud {
            @display("i=abstract/switch;p=243,131,m,8,80,80");
        }

        Rks[accessswitch]: Racks {
            @display("p=58,342,m,16,80,80;i=device/router");
        }





    connections allowunconnected:
        for i=0..corerouter-1, for j=0..aggregateswitch-1 {
            CS1[i].ethg++ <--> Eth100M <--> AS1[j].ethg++;
           // CS1[i].ethg++ <-->  AS1[j].ethg++;
        }
	//	for i=0..aggregateswitch-1, for j=i*2..2*(i+1)-1 {
		    
		 //   Rks[j].iogate++ <--> Eth100M <--> AS1[i].ethg++;
		    
		    
		//}
		
}
