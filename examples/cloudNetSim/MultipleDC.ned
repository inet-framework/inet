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

import inet.node.ethernet.Eth10G;
import inet.node.ethernet.Eth10M;
import inet.examples.ethernet.lans.cable;
import inet.node.ethernet.Eth100G;
import inet.greencloudsimulator.Server;
import inet.greencloudsimulator.AccessSwitch;
import inet.greencloudsimulator.AggregateSwitch;
import inet.greencloudsimulator.CoreSwitch;

import inet.greencloudsimulator.myStandardHost;
import inet.greencloudsimulator.myComputingNode;
import inet.greencloudsimulator.myLocalSchedular;

import inet.greencloudsimulator.RouterGreenCloud;

import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import inet.node.ethernet.EtherHost;
import inet.node.ethernet.EtherSwitch;
import ned.DatarateChannel;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.networklayer.configurator.ipv4.Ipv4FlatNetworkConfigurator;

import inet.node.ethernet.Eth1G;

module RACK
{
    parameters:
        int N @prompt("Nodes per rack");// = default(1);
        @display("bgb=954,473");
    gates:
        inout iogate[];
    submodules:
        computingServer[N]: myComputingNode {
            @display("i=device/server2;p=64,298,m,8,80,80;i2=status/connect,red");
        }
        accessRouter: RouterGreenCloud {
            @display("p=269,159");
        }

    connections allowunconnected:
        for i=0..N-1 {
            accessRouter.ethg++ <--> Eth10G {  ber = default(1e-6); length = default(1km); } <--> computingServer[i].ethg++;
        }
        accessRouter.ethg++ <--> iogate++;
        //accessRouter.ethg++ <--> iogate++;// based on racks input from datacenter
        //accessRouter.ethg++ <--> iogate++;
}

network MultipleDatacenter
{
    parameters:
        @display("bgb=1074,688");
        int aggregateswitch = default(4);
        int corerouter = default(2);
        int accessswitch = default(16);
        int aggregateswitch2 = default(4);
        int corerouter2 = default(2);
        int accessswitch2 = default(16);

    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=887,17");
        }
        AS1[aggregateswitch]: Router {
            @display("i=abstract/router2;p=122,494,r,300,80,80");
        }

        CS1[corerouter]: Router {
            @display("i=abstract/switch;p=293,402,r,300,80,80");
        }

        Rks[accessswitch]: RACK {
            @display("p=25,623,m,16,80,80;i=device/router");
        }

        AS2[aggregateswitch2]: Router {
            @display("i=abstract/router2;p=122,129,r,300,80,80");
        }

        CS2[corerouter2]: Router {
            @display("i=abstract/switch;p=293,221,r,300,80,80");
        }

        Rks2[accessswitch2]: RACK {
            @display("p=25,17,m,16,80,80;i=device/router");
        }



        Central: Router {
            @display("i=abstract/switch;p=419,319,r,300,80,80");
        }


        Schedular: myLocalSchedular {
            @display("p=60,319;i=device/router");
        }
        User: myStandardHost {
            @display("p=602,319");
        }
    connections allowunconnected:

        for i=0..corerouter-1, for j=0..aggregateswitch-1 {
            CS1[i].ethg++ <--> Eth10G {  ber = default(1e-5); } <--> AS1[j].ethg++;
            CS1[i].ethg++ <--> Eth10G {  ber = default(1e-5); } <--> Central.ethg++;
        }
        //0-15
        for j=0..3 {
            AS1[0].ethg++ <--> Eth1G {  ber = default(1e-5); } <--> Rks[j].iogate++;
        }
        //16-31
        for j=4..7 {
            AS1[1].ethg++ <--> Eth1G {  ber = default(1e-5); } <--> Rks[j].iogate++;
        }
        for j=8..11 {
            AS1[2].ethg++ <--> Eth1G {  ber = default(1e-5); } <--> Rks[j].iogate++;
        }
        for j=12..15 {
            AS1[3].ethg++ <--> Eth1G {  ber = default(1e-5); } <--> Rks[j].iogate++;
        }
        // DC SECOND
        for i=0..corerouter-1, for j=0..aggregateswitch-1 {
            CS2[i].ethg++ <--> Eth10G {  ber = default(1e-5); } <--> AS2[j].ethg++;
            CS2[i].ethg++ <--> Eth10G {  ber = default(1e-5); } <--> Central.ethg++;
        }
        //0-15
        for j=0..3 {
            AS2[0].ethg++ <--> Eth1G {  ber = default(1e-5); } <--> Rks2[j].iogate++;
        }
        //16-31
        for j=4..7 {
            AS2[1].ethg++ <--> Eth1G {  ber = default(1e-5); } <--> Rks2[j].iogate++;
        }
        for j=8..11 {
            AS2[2].ethg++ <--> Eth1G {  ber = default(1e-5); } <--> Rks2[j].iogate++;
        }
        for j=12..15 {
            AS2[3].ethg++ <--> Eth1G {  ber = default(1e-5); } <--> Rks2[j].iogate++;
        }

        Schedular.ethg++ <--> Eth1G {  ber = default(1e-5); } <-->  Central.ethg++;
        User.ethg++ <--> Eth1G {  ber = default(1e-5); } <-->  Central.ethg++;
}


network MultipleDatacenterExt
{
    parameters:
        @display("bgb=1074,688");
        int aggregateswitch = default(16);
        int corerouter = default(8);
        int accessswitch = default(16);
        int aggregateswitch2 = default(16);
        int corerouter2 = default(2);
        int accessswitch2 = default(16);

    submodules:
        
        configurator: Ipv4NetworkConfigurator {
            @display("p=887,17");
        }        
        AS1[aggregateswitch]: Router {
            @display("i=abstract/router2;p=122,494,r,300,80,80");
        }

        CS1[corerouter]: Router {
            @display("i=abstract/switch;p=293,402,r,300,80,80");
        }

        Rks[accessswitch]: RACK {
            @display("p=25,623,m,16,80,80;i=device/router");
        }

        AS2[aggregateswitch2]: Router {
            @display("i=abstract/router2;p=122,129,r,300,80,80");
        }

        CS2[corerouter2]: Router {
            @display("i=abstract/switch;p=293,221,r,300,80,80");
        }

        Rks2[accessswitch2]: RACK {
            @display("p=25,17,m,16,80,80;i=device/router");
        }



        Central: Router {
            @display("i=abstract/switch;p=419,319,r,300,80,80");
        }


        Schedular: myLocalSchedular {
            @display("p=60,319;i=device/router");
        }
        User: myStandardHost {
            @display("p=602,319");
        }
    connections allowunconnected:

        for i=0..corerouter-1, for j=0..aggregateswitch-1 {
            CS1[i].ethg++ <--> Eth10G {  ber = default(1e-5); } <--> AS1[j].ethg++;
            CS1[i].ethg++ <--> Eth10G {  ber = default(1e-5); } <--> Central.ethg++;
        }
        //0-15
        for j=0..3 {
            AS1[0].ethg++ <--> Eth1G {  ber = default(1e-5); } <--> Rks[j].iogate++;
        }
        //16-31
        for j=4..7 {
            AS1[1].ethg++ <--> Eth1G {  ber = default(1e-5); } <--> Rks[j].iogate++;
        }
        for j=8..11 {
            AS1[2].ethg++ <--> Eth1G {  ber = default(1e-5); } <--> Rks[j].iogate++;
        }
        for j=12..15 {
            AS1[3].ethg++ <--> Eth1G {  ber = default(1e-5); } <--> Rks[j].iogate++;
        }
        // DC SECOND
        for i=0..corerouter-1, for j=0..aggregateswitch-1 {
            CS2[i].ethg++ <--> Eth10G {  ber = default(1e-5); } <--> AS2[j].ethg++;
            CS2[i].ethg++ <--> Eth10G {  ber = default(1e-5); } <--> Central.ethg++;
        }
        //0-15
        for j=0..3 {
            AS2[0].ethg++ <--> Eth1G {  ber = default(1e-5); } <--> Rks2[j].iogate++;
        }
        //16-31
        for j=4..7 {
            AS2[1].ethg++ <--> Eth1G {  ber = default(1e-5); } <--> Rks2[j].iogate++;
        }
        for j=8..11 {
            AS2[2].ethg++ <--> Eth1G {  ber = default(1e-5); } <--> Rks2[j].iogate++;
        }
        for j=12..15 {
            AS2[3].ethg++ <--> Eth1G {  ber = default(1e-5); } <--> Rks2[j].iogate++;
        }

        Schedular.ethg++ <--> Eth1G {  ber = default(1e-5); } <-->  Central.ethg++;
        User.ethg++ <--> Eth1G {  ber = default(1e-5); } <-->  Central.ethg++;
}

