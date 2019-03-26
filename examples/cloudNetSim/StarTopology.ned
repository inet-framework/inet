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
import inet.node.ethernet.Eth100M;
import inet.node.ethernet.EtherLink;
import ned.DelayChannel;
import inet.greencloudsimulator.RouterGreenCloud;
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


network StarTopology // with increased nodes
{
    @display("bgi=maps/england;bgb=957,510");
    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=887,17");
        }
        
        North: CDatacenterUpdated {
            @display("i=device/lan-ring;p=204,66");
        }
        East: CDatacenterUpdated {
            @display("p=353,257;i=device/lan-ring");
        }
        West: CDatacenterUpdated {
            @display("i=device/lan-ring;p=180,257");
        }
        South: CDatacenterUpdated {
            @display("p=337,392;i=device/lan-ring");
        }
        Rout: Router {
            @display("p=258,209;i=misc/cloud");
        }
        R1: Router {
            @display("p=219,107");
        }
        R2: Router {
            @display("p=314,226");
        }
        R3: Router {
            @display("p=219,257");
        }
        R4: Router {
            @display("p=308,340");
        }

    connections allowunconnected:
        North.iogate++ <--> Eth10G <--> R1.ethg++;
        North.iogate++ <--> Eth10G <--> R1.ethg++;
        North.iogate++ <--> Eth10G <--> R1.ethg++;
        North.iogate++ <--> Eth10G <--> R1.ethg++;
        East.iogate++ <--> Eth10G <--> R2.ethg++;
        East.iogate++ <--> Eth10G <--> R2.ethg++;
        East.iogate++ <--> Eth10G <--> R2.ethg++;
        East.iogate++ <--> Eth10G <--> R2.ethg++;
        West.iogate++ <--> Eth10G <--> R3.ethg++;
        West.iogate++ <--> Eth10G <--> R3.ethg++;
        West.iogate++ <--> Eth10G <--> R3.ethg++;
        West.iogate++ <--> Eth10G <--> R3.ethg++;
        South.iogate++ <--> Eth10G <--> R4.ethg++;
        South.iogate++ <--> Eth10G <--> R4.ethg++;
        South.iogate++ <--> Eth10G <--> R4.ethg++;
        South.iogate++ <--> Eth10G <--> R4.ethg++;

        R1.ethg++ <--> Eth10G {  ber = default(1e-12); length = default(1km); } <--> Rout.ethg++;
        R2.ethg++ <--> Eth10G {  ber = default(1e-12); length = default(1km); } <--> Rout.ethg++;
        R3.ethg++ <--> Eth10G {  ber = default(1e-12); length = default(1km); } <--> Rout.ethg++;
        R4.ethg++ <--> Eth10G {  ber = default(1e-12); length = default(1km); } <--> Rout.ethg++;
}

module CDatacenterUpdated
{
    parameters:
        int servercount = default(2);//@prompt("Number of Racks ") = default(2);
        int aggregateswitch = default(16);
        int corerouter = default(8);
        int accessswitch = default(256);
        //int ins @prompt("Number of TaskGenerator");
        @display("bgb=954,456");
    gates:
        inout iogate[];

    submodules:

        AS1[aggregateswitch]: RouterGreenCloud {
            @display("i=abstract/router2;p=138,220,m,8,80,80");
        }

        CS1[corerouter]: RouterGreenCloud {
            @display("i=abstract/switch;p=243,131,m,8,80,80");
        }

        Rks[accessswitch]: DisRacks {
            @display("p=58,342,m,10,80,80;i=device/router");
        }
    connections allowunconnected:

        CS1[0].ethg++ <--> iogate++;
        CS1[1].ethg++ <--> iogate++;
        CS1[2].ethg++ <--> iogate++;
        CS1[3].ethg++ <--> iogate++;

        for i=0..corerouter-1, for j=0..aggregateswitch-1 {
            CS1[i].ethg++ <--> Eth10G {  ber = default(1e-12); } <--> AS1[j].ethg++;
        }
        //0-15
        for j=0..15 {
            AS1[0].ethg++ <--> Eth10G {  ber = default(1e-12); } <--> Rks[j].iogate++;
        }
        //16-31
        for j=16..31 {
            AS1[1].ethg++ <--> Eth10G {  ber = default(1e-12); } <--> Rks[j].iogate++;
        }
        for j=32..47 {
            AS1[2].ethg++ <--> Eth10G {  ber = default(1e-12); } <--> Rks[j].iogate++;
        }
        for j=48..63 {
            AS1[3].ethg++ <--> Eth10G {  ber = default(1e-12); } <--> Rks[j].iogate++;
        }
        for j=64..79 {
            AS1[4].ethg++ <--> Eth10G {  ber = default(1e-12); } <--> Rks[j].iogate++;
        }
        for j=80..95 {
            AS1[5].ethg++ <--> Eth10G {  ber = default(1e-12); } <--> Rks[j].iogate++;
        }
        for j=96..111 {
            AS1[6].ethg++ <--> Eth10G {  ber = default(1e-12); } <--> Rks[j].iogate++;
        }
        for j=112..127 {
            AS1[7].ethg++ <--> Eth10G {  ber = default(1e-12); } <--> Rks[j].iogate++;
        }
        for j=128..143 {
            AS1[8].ethg++ <--> Eth10G {  ber = default(1e-12); } <--> Rks[j].iogate++;
        }
        for j=144..159 {
            AS1[9].ethg++ <--> Eth10G {  ber = default(1e-12); } <--> Rks[j].iogate++;
        }
        for j=160..175 {
            AS1[10].ethg++ <--> Eth10G {  ber = default(1e-12); } <--> Rks[j].iogate++;
        }
        for j=176..191 {
            AS1[11].ethg++ <--> Eth10G {  ber = default(1e-12); } <--> Rks[j].iogate++;
        }
        for j=192..207 {
            AS1[12].ethg++ <--> Eth10G {  ber = default(1e-12); } <--> Rks[j].iogate++;
        }
        for j=208..223 {
            AS1[13].ethg++ <--> Eth10G {  ber = default(1e-12); } <--> Rks[j].iogate++;
        }
        for j=224..239 {
            AS1[14].ethg++ <--> Eth10G {  ber = default(1e-12); } <--> Rks[j].iogate++;
        }
        for j=240..255 {
            AS1[15].ethg++ <--> Eth10G {  ber = default(1e-12); } <--> Rks[j].iogate++;
        }

}


module DisRacks
{
    parameters:
        int N @prompt("Nodes per rack");// = default(1);
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
            accessRouter.ethg++ <--> Eth10G {  ber = default(1e-6); length = default(1km); }<--> computingServer[i].ethg++;
        }
        accessRouter.ethg++ <--> iogate++;
        //accessRouter.ethg++ <--> iogate++;// based on racks input from datacenter
        //accessRouter.ethg++ <--> iogate++;
        //accessRouter.ethg++ <--> iogate++;
}
