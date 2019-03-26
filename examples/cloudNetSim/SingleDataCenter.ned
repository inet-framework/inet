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

package greencloudsimulator.simulations;

import inet.node.ethernet.Eth10G;
import inet.node.ethernet.Eth10M;
import inet.examples.ethernet.lans.cable;
import inet.node.ethernet.Eth100G;
import inet.greencloudsimulator.Server;
import inet.greencloudsimulator.AccessSwitch;
import inet.greencloudsimulator.AggregateSwitch;
import inet.greencloudsimulator.CoreSwitch;
import inet.greencloudsimulator.myStandardHost;


import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import inet.node.ethernet.EtherHost;
import inet.node.ethernet.EtherSwitch;
import ned.DatarateChannel;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.networklayer.configurator.ipv4.Ipv4FlatNetworkConfigurator;

import inet.node.ethernet.Eth1G;

module rack
{
    parameters:
        //int N @prompt("Nodes per rack") = default(1);
        @display("bgb=506,467");
    gates:
        inout iogate[];
    submodules:
        computingServer[1]: myStandardHost {
            @display("i=device/server2;p=64,298,m,8,80,80");
        }
        accessRouter: Router {
            @display("p=202,148");
        }

    connections allowunconnected:
        //for i=0..N-1 {
        accessRouter.ethg++ <--> Eth1G <--> computingServer[0].ethg++;
        //   accessRouter.ethg++ <--> Eth1G <--> computingServer[1].ethg++;
        // }
        //for j=0..1 {
        accessRouter.ethg++ <--> iogate++;
        accessRouter.ethg++ <--> iogate++;
        //}
       // accessRouter.ethg++ <--> iogate++;// based on racks input from datacenter

}

network SDC
{
    parameters:
        @display("bgb=954,456");
        int servercount = default(2);//@prompt("Number of Racks ") = default(2);
        int aggregateswitch = default(1);
        int corerouter = default(1);
        int accessswitch = default(1);

    submodules:
        
        configurator: Ipv4NetworkConfigurator {
            @display("p=887,17");
        }
        
        MainR: Router {
            @display("i=abstract/router2;p=281,87,m,8,80,80");
        }

        InterR: Router {
            @display("i=abstract/switch;p=281,178,m,8,80,80");
        }


        H1: myStandardHost {
            @display("p=71,279,m,16,80,80;i=device/router");
        }
        H2: myStandardHost {
            @display("p=215,279,m,16,80,80;i=device/router");
        }

        H3: myStandardHost {
            @display("p=336,279,m,16,80,80;i=device/router");
        }

        H4: myStandardHost {
            @display("p=443,279,m,16,80,80;i=device/router");
        }

        A1: myStandardHost {
            @display("p=617,130,m,16,80,80;i=device/router");
        }
        A2: myStandardHost {
            @display("p=595,48,m,16,80,80;i=device/router");
        }



    connections allowunconnected:

        // for i=0..corerouter-1, for j=0..aggregateswitch-1 {
        MainR.pppg++ <--> Eth10G {  ber = default(1e-5); } <--> InterR.pppg++;
        //}

        InterR.pppg++ <--> Eth1G {  ber = default(1000e-5); } <--> H1.pppg++;
        InterR.pppg++ <--> Eth1G {  ber = default(1e-5); } <--> H2.pppg++;
        InterR.pppg++ <--> Eth1G {  ber = default(1e-5); } <--> H3.pppg++;
        InterR.pppg++ <--> Eth1G {  ber = default(1e-5); } <--> H4.pppg++;


        MainR.ethg++ <--> Eth1G {  ber = default(1e-5); } <--> A1.ethg++;
        MainR.ethg++ <--> Eth1G {  ber = default(1e-5); } <--> A2.ethg++;


       //   AS1[0].ethg++ <--> Eth1G  {  ber = default(1e-5); }<--> Rks[1].iogate++;

      //    AS1[1].ethg++ <--> Eth1G  {  ber = default(1e-5); }<--> Rks[2].iogate++;
      //    AS1[1].ethg++ <--> Eth1G  {  ber = default(1e-5); }<--> Rks[3].iogate++;

      //    AS1[2].ethg++ <--> Eth1G  {  ber = default(1e-5); }<--> Rks[4].iogate++;
      //    AS1[2].ethg++ <--> Eth1G  {  ber = default(1e-5); }<--> Rks[5].iogate++;

      //    AS1[3].ethg++ <--> Eth1G  {  ber = default(1e-5); }<--> Rks[6].iogate++;
      //    AS1[3].ethg++ <--> Eth1G  {  ber = default(1e-5); }<--> Rks[7].iogate++;


}



