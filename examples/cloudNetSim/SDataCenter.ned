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

package inet.examples.cloudNetSim;;

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

module Racksi
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

    connections:
        for i=0..N-1 {
            accessRouter.ethg++ <--> Eth10G {  ber = default(1e-6); length = default(1km); } <--> computingServer[i].ethg++;
        }
        accessRouter.ethg++ <--> iogate++;
       // accessRouter.ethg++ <--> iogate++;// based on racks input from datacenter
        //accessRouter.ethg++ <--> iogate++;
}

network SDatacenter
{
    parameters:
        @display("bgb=1074,458");
        int aggregateswitch = default(4);
        int corerouter = default(2);
        int accessswitch = default(16);

    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=887,17");
        }
        
        AS1[aggregateswitch]: Router {
            @display("i=abstract/router2;p=161,291,r,300,80,80");
        }

        CS1[corerouter]: Router {
            @display("i=abstract/switch;p=455,200,r,300,80,80");
        }

        Rks[accessswitch]: Racksi {
            @display("p=21,420,m,16,80,80;i=device/router");
        }


        Schedular: myLocalSchedular {
            @display("p=465,107;i=device/router");
        }
        User: myStandardHost {
            @display("p=650,95");
        }
    connections allowunconnected:

        for i=0..corerouter-1, for j=0..aggregateswitch-1 {
            CS1[i].ethg++ <--> Eth10G {  ber = default(1e-5); } <--> AS1[j].ethg++;
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

        CS1[0].ethg++ <--> Eth1G {  ber = default(1e-5); } <--> Schedular.ethg++;
        CS1[0].ethg++ <--> Eth1G {  ber = default(1e-5); } <--> User.ethg++;
}

network SDatacenterExt
{
    parameters:
        @display("bgb=1074,458");
        int aggregateswitch = default(8);
        int corerouter = default(4);
        int accessswitch = default(128);

    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=887,17");
        }
        
        AS1[aggregateswitch]: Router {
            @display("i=abstract/router2;p=161,291,r,300,80,80");
        }

        CS1[corerouter]: Router {
            @display("i=abstract/switch;p=455,200,r,300,80,80");
        }

        Rks[accessswitch]: Racksi {
            @display("p=21,420,m,16,80,80;i=device/router");
        }


        Schedular: myLocalSchedular {
            @display("p=465,107;i=device/router");
        }
        User: myStandardHost {
            @display("p=650,95");
        }
    connections allowunconnected:

        for i=0..corerouter-1, for j=0..aggregateswitch-1 {
            CS1[i].ethg++ <--> Eth10G {  ber = default(1e-5); } <--> AS1[j].ethg++;
        }
        //AGG
        //0-15
        for j=0..15 {
            AS1[0].ethg++ <--> Eth1G {  ber = default(1e-5); } <--> Rks[j].iogate++;
        }
        for j=0..15 {
            AS1[1].ethg++ <--> Eth1G {  ber = default(1e-5); } <--> Rks[j].iogate++;
        }

        for j=16..31 {
            AS1[1].ethg++ <--> Eth1G {  ber = default(1e-5); } <--> Rks[j].iogate++;
        }
        for j=16..31 {
            AS1[0].ethg++ <--> Eth1G {  ber = default(1e-5); } <--> Rks[j].iogate++;
        }

        for j=32..47 {
            AS1[2].ethg++ <--> Eth1G {  ber = default(1e-5); } <--> Rks[j].iogate++;
        }
        for j=32..47 {
            AS1[3].ethg++ <--> Eth1G {  ber = default(1e-5); } <--> Rks[j].iogate++;
        }
        for j=48..63 {
            AS1[3].ethg++ <--> Eth1G {  ber = default(1e-5); } <--> Rks[j].iogate++;
        }
        for j=48..63 {
            AS1[2].ethg++ <--> Eth1G {  ber = default(1e-5); } <--> Rks[j].iogate++;
        }

        for j=64..79 {
            AS1[4].ethg++ <--> Eth1G {  ber = default(1e-5); } <--> Rks[j].iogate++;
        }
        for j=80..95 {
            AS1[5].ethg++ <--> Eth1G {  ber = default(1e-5); } <--> Rks[j].iogate++;
        }
        for j=64..79 {
            AS1[5].ethg++ <--> Eth1G {  ber = default(1e-5); } <--> Rks[j].iogate++;
        }
        for j=80..95 {
            AS1[4].ethg++ <--> Eth1G {  ber = default(1e-5); } <--> Rks[j].iogate++;
        }

        for j=96..111 {
            AS1[6].ethg++ <--> Eth1G {  ber = default(1e-5); } <--> Rks[j].iogate++;
        }
        for j=112..127 {
            AS1[7].ethg++ <--> Eth1G {  ber = default(1e-5); } <--> Rks[j].iogate++;
        }
        for j=96..111 {
            AS1[7].ethg++ <--> Eth1G {  ber = default(1e-5); } <--> Rks[j].iogate++;
        }
        for j=112..127 {
            AS1[6].ethg++ <--> Eth1G {  ber = default(1e-5); } <--> Rks[j].iogate++;
        }




        CS1[0].ethg++ <--> Eth1G {  ber = default(1e-5); } <--> Schedular.ethg++;
        CS1[0].ethg++ <--> Eth1G {  ber = default(1e-5); } <--> User.ethg++;
}

