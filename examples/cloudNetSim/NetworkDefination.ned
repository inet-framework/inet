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
import inet.greencloudsimulator.myComputingNode;
import inet.greencloudsimulator.myLocalSchedular;
import inet.node.ethernet.Eth1G;


module Racks
{
    parameters:
        int N @prompt("Nodes per rack");// = default(1);
        @display("bgb=506,467");
    gates:
        inout iogate[];
    submodules:
        computingServer[N]: myComputingNode {
            @display("i=device/server2;p=64,298,m,8,80,80");
        }
        accessRouter: RouterGreenCloud {
            @display("p=202,148");
        }
    connections:
        for i=0..N-1 {
            accessRouter.ethg++ <--> Eth10G {  ber = default(1e-6); length = default(1km); } <--> computingServer[i].ethg++;
        }
        accessRouter.ethg++ <--> iogate++;
        accessRouter.ethg++ <--> iogate++;// based on racks input from datacenter
       // accessRouter.ethg++ <--> iogate++;
}

network DistributedMeshTopology
{
    @display("bgi=maps/england;bgb=957,510");
    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=887,17");
        }
        Schedular: myLocalSchedular {
            @display("p=348,173;i=device/router");
        }
        User: myStandardHost {
            @display("p=508,87");
        }
        North: MeshCDatacenter {
            @display("i=device/lan-ring;p=204,66");
        }
        East: MeshCDatacenter {
            @display("p=401,267;i=device/lan-ring");
        }
        West: MeshCDatacenter {
            @display("i=device/lan-ring;p=144,288");
        }
        South: MeshCDatacenter {
            @display("p=315,412;i=device/lan-ring");
        }

        R1: Router {
            @display("p=226,124;is=s");
        }
       R2: Router {
           @display("p=354,267;is=s");
       }
       R3: Router {
           @display("p=198,279;is=s");
       }
       R4: Router {
           @display("p=306,371;is=s");
       }

        

    connections allowunconnected:
        North.iogate++ <--> R1.ethg++;
        North.iogate++ <--> R1.ethg++;
        North.iogate++ <--> R1.ethg++;
        North.iogate++ <--> R1.ethg++;
        East.iogate++ <--> R2.ethg++;
        East.iogate++ <--> R2.ethg++;
        East.iogate++ <--> R2.ethg++;
        East.iogate++ <--> R2.ethg++;
        West.iogate++ <--> R3.ethg++;
        West.iogate++ <--> R3.ethg++;
        West.iogate++ <--> R3.ethg++;
        West.iogate++ <--> R3.ethg++;
        South.iogate++ <--> R4.ethg++;
        South.iogate++ <--> R4.ethg++;
        South.iogate++ <--> R4.ethg++;
        South.iogate++ <--> R4.ethg++;

        R1.ethg++ <--> Eth100G {  ber = default(1e-5); length = default(10m); } <--> R2.ethg++;
        R2.ethg++ <--> Eth100G {  ber = default(1e-5); length = default(10m); } <--> R3.ethg++;
        R3.ethg++ <--> Eth100G {  ber = default(1e-5); length = default(10m); } <--> R4.ethg++;
        R2.ethg++ <--> Eth100G {  ber = default(1e-5); length = default(10m); } <--> R4.ethg++;
        R3.ethg++ <--> Eth100G {  ber = default(1e-5); length = default(10m); } <--> R1.ethg++;
        R4.ethg++ <--> Eth100G {  ber = default(1e-5); length = default(10m); } <--> R1.ethg++;
        Schedular.ethg++ <--> Eth100G {  ber = default(1e-5); length = default(10m); } <--> R1.ethg++;
       Schedular.ethg++ <--> Eth100G {  ber = default(1e-5); length = default(10m); } <--> R2.ethg++;
       Schedular.ethg++ <--> Eth100G {  ber = default(1e-5); length = default(10m); } <--> R3.ethg++;
       Schedular.ethg++ <--> Eth100G {  ber = default(1e-5); length = default(10m); } <--> R4.ethg++;
       User.ethg++ <--> Eth100G {  ber = default(1e-5); length = default(10m); } <--> Schedular.ethg++;
}
module MeshCDatacenter
{
    parameters:
        int servercount = default(2);//@prompt("Number of Racks ") = default(2);
        int aggregateswitch = default(8);
        int corerouter = default(4);
        int accessswitch = default(8);
        //int ins @prompt("Number of TaskGenerator");
        @display("bgb=954,456");
    gates:
        inout iogate[];

    submodules:

        SiteSchedular: myLocalSchedular {
            @display("p=440,48;i=device/router");
        }

        AS1[aggregateswitch]: RouterGreenCloud {
            @display("i=abstract/router2;p=138,220,m,8,80,80");
        }

        CS1[corerouter]: RouterGreenCloud {
            @display("i=abstract/switch;p=243,131,m,8,80,80");
        }

        Rks[accessswitch]: Racks {
            @display("p=58,342,m,10,80,80;i=device/router");
        }


     //   configurator: FlatNetworkConfigurator {
     //      @display("p=887,17");
     //  }


    connections allowunconnected:
        for i=0..corerouter-1, for j=0..aggregateswitch-1 {
            CS1[i].ethg++ <--> Eth10G <--> AS1[j].ethg++;
           // CS1[i].ethg++ <-->  AS1[j].ethg++;
        }

        CS1[0].ethg++ <--> Eth10G <--> iogate++;
        //CS1[1].ethg++ <--> Eth10G <--> iogate++;
        //CS1[2].ethg++ <--> Eth10G <--> iogate++;
        //CS1[3].ethg++ <--> Eth10G <--> iogate++;

        Rks[0].iogate++ <--> Eth10G <--> AS1[0].ethg++;
        Rks[0].iogate++ <--> Eth10G <--> AS1[1].ethg++;

        Rks[1].iogate++ <--> Eth10G <--> AS1[0].ethg++;
        Rks[1].iogate++ <--> Eth10G <--> AS1[1].ethg++;

        Rks[2].iogate++ <--> Eth10G <--> AS1[2].ethg++;
        Rks[2].iogate++ <--> Eth10G <--> AS1[3].ethg++;

        Rks[3].iogate++ <--> Eth10G <--> AS1[2].ethg++;
        Rks[3].iogate++ <--> Eth10G <--> AS1[3].ethg++;

        Rks[4].iogate++ <--> Eth10G <--> AS1[4].ethg++;
        Rks[4].iogate++ <--> Eth10G <--> AS1[5].ethg++;

        Rks[5].iogate++ <--> Eth10G <--> AS1[4].ethg++;
        Rks[5].iogate++ <--> Eth10G <--> AS1[5].ethg++;

        Rks[6].iogate++ <--> Eth10G <--> AS1[6].ethg++;
        Rks[6].iogate++ <--> Eth10G <--> AS1[7].ethg++;

        Rks[7].iogate++ <--> Eth10G <--> AS1[6].ethg++;
        Rks[7].iogate++ <--> Eth10G <--> AS1[7].ethg++;


//        Rks[1].iogate++ <--> cable <--> AS1[1].ethg++;
  //      Rks[2].iogate++ <--> cable <--> AS1[1].ethg++;



//        for i=2..3, for j=2..3 {
        //    AS1[i].ethg++ <--> cable <--> AcS1[j].ethg++;

  //      }


        SiteSchedular.ethg++ <--> Eth10G <--> CS1[0].ethg++;
}


network DistributedDatacenter
{
    @display("bgi=maps/england;bgb=957,510");
    submodules:
        North: CDatacenter {
            @display("i=device/lan-ring;p=204,66");
        }
        East: CDatacenter {
            @display("p=353,257;i=device/lan-ring");
        }
        West: CDatacenter {
            @display("i=device/lan-ring;p=180,257");
        }
        South: CDatacenter {
            @display("p=337,392;i=device/lan-ring");
        }
        Rout: Router {
            @display("p=258,209;i=misc/cloud");
        }
        R1: Router {
            @display("p=219,107;is=s");
        }
        R2: Router {
            @display("p=314,226;is=s");
        }
        R3: Router {
            @display("p=219,257;is=s");
        }
        R4: Router {
            @display("p=308,340;is=s");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=887,17");
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

module CDatacenter
{
    parameters:
        int servercount = default(2);//@prompt("Number of Racks ") = default(2);
        int aggregateswitch = default(8);
        int corerouter = default(4);
        int accessswitch = default(8);
        int ins @prompt("Number of TaskGenerator");
        @display("bgb=954,456");
    gates:
        inout iogate[];

    submodules:

//ControlPanel: StandardHost {
  //          @display("p=21,220;i=block/plug");
   //     }

       // TskGen[ins]: myStandardHost {
       //     @display("p=372,17,m,15,80,80;i=block/users");
      //  }
        //RemoteAdmin: myStandardHost {
        //    @display("p=50,73");
       // }

      //  PublicRouter: RouterGreenCloud {
        //    @display("p=313,69");
       // }

        AS1[aggregateswitch]: RouterGreenCloud {
            @display("i=abstract/router2;p=138,220,m,8,80,80");
        }

        CS1[corerouter]: RouterGreenCloud {
            @display("i=abstract/switch;p=243,131,m,8,80,80");
        }

        Rks[accessswitch]: Racks {
            @display("p=58,342,m,10,80,80;i=device/router");
        }


        //configurator: FlatNetworkConfigurator {
          //  @display("p=887,17");
        //}


    connections allowunconnected:
        for i=0..corerouter-1, for j=0..aggregateswitch-1 {
            CS1[i].ethg++ <--> Eth100M <--> AS1[j].ethg++;
           // CS1[i].ethg++ <-->  AS1[j].ethg++;
        }

        CS1[0].ethg++ <--> iogate++;
        CS1[1].ethg++ <--> iogate++;
        CS1[2].ethg++ <--> iogate++;
        CS1[3].ethg++ <--> iogate++;

       // for k=0..ins-1 {
       //     TskGen[k].ethg++ <--> Eth10M <--> PublicRouter.ethg++;

        //}
        //PublicRouter.ethg++ <--> Eth10M <--> CS1[1].ethg++;
        //Intruder[0].pppg++ <--> Eth10M <--> PublicRouter.pppg++;
        //Intruder[1].pppg++ <--> Eth10M <--> PublicRouter.pppg++;
        //Intruder3.pppg++ <--> Eth10M <--> PublicRouter.pppg++;
        //RemoteAdmin.pppg++ <--> Eth10M <--> PublicRouter.pppg++;
        //ControlPanel.pppg++ <--> Eth10M <--> CS1[1].pppg++;
    //    for i=0..1, for j=0..1 {
      //      Rks[i].iogate++ <--> cable <--> AS1[j].ethg++;

        //}
        Rks[0].iogate++ <--> Eth100M {  ber = default(1e-12); length = default(1km); } <--> AS1[0].ethg++;
        Rks[0].iogate++ <--> Eth100M {  ber = default(1e-12); length = default(1km); } <--> AS1[1].ethg++;

        Rks[1].iogate++ <--> Eth100M {  ber = default(1e-12); length = default(1km); } <--> AS1[0].ethg++;
        Rks[1].iogate++ <--> Eth100M {  ber = default(1e-12); length = default(1km); } <--> AS1[1].ethg++;

        Rks[2].iogate++ <--> Eth100M {  ber = default(1e-12); length = default(1km); } <--> AS1[2].ethg++;
        Rks[2].iogate++ <--> Eth100M {  ber = default(1e-12); length = default(1km); } <--> AS1[3].ethg++;

        Rks[3].iogate++ <--> Eth100M {  ber = default(1e-12); length = default(1km); } <--> AS1[2].ethg++;
        Rks[3].iogate++ <--> Eth100M {  ber = default(1e-12); length = default(1km); } <--> AS1[3].ethg++;

        Rks[4].iogate++ <--> Eth100M {  ber = default(1e-12); length = default(1km); } <--> AS1[4].ethg++;
        Rks[4].iogate++ <--> Eth100M {  ber = default(1e-12); length = default(1km); } <--> AS1[5].ethg++;

        Rks[5].iogate++ <--> Eth100M {  ber = default(1e-12); length = default(1km); } <--> AS1[4].ethg++;
        Rks[5].iogate++ <--> Eth100M {  ber = default(1e-12); length = default(1km); } <--> AS1[5].ethg++;

        Rks[6].iogate++ <--> Eth100M {  ber = default(1e-12); length = default(1km); } <--> AS1[6].ethg++;
        Rks[6].iogate++ <--> Eth100M {  ber = default(1e-12); length = default(1km); } <--> AS1[7].ethg++;

        Rks[7].iogate++ <--> Eth100M {  ber = default(1e-12); length = default(1km); } <--> AS1[6].ethg++;
        Rks[7].iogate++ <--> Eth100M {  ber = default(1e-12); length = default(1km); } <--> AS1[7].ethg++;


//        Rks[1].iogate++ <--> cable <--> AS1[1].ethg++;
  //      Rks[2].iogate++ <--> cable <--> AS1[1].ethg++;



//        for i=2..3, for j=2..3 {
        //    AS1[i].ethg++ <--> cable <--> AcS1[j].ethg++;

  //      }


}

network CDatacenterTopology
{
    parameters:
        int servercount = default(2);//@prompt("Number of Racks ") = default(2);
        int aggregateswitch = default(8);
        int corerouter = default(8);
        int accessswitch = default(16);
        int ins @prompt("Number of TaskGenerator");
        @display("bgb=954,456");
    gates:
        inout iogate[];

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
		//for j=0..corerouter-1 {
		 //   CS1[j].ethg++ <--> iogate++;
		//}

        for i=0..aggregateswitch-1, for j=i*2..2*(i+1)-1 {
            Rks[j].iogate++ <--> Eth100M <--> AS1[i].ethg++;


        }



}


network Datacenter
{
    parameters:
        int servercount = default(2);//@prompt("Number of Racks ") = default(2);
        int aggregateswitch = default(8);
        int corerouter = default(4);
        int accessswitch = default(8);

        @display("bgb=954,456");
    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=887,17");
        }        
        AS1[aggregateswitch]: Router {
            @display("i=abstract/router2;p=161,139,m,8,80,80");
        }

        CS1[corerouter]: Router {
            @display("i=abstract/switch;p=311,40,m,8,80,80");
        }

        AcS1[accessswitch]: Router {
            @display("p=71,279,m,10,80,80");
        }

        ServerNode[servercount*8]: myStandardHost {
            @display("i=device/server2;p=37,378,m,16,50,50");
        }

    connections allowunconnected:
        for i=0..corerouter-1, for j=0..aggregateswitch-1 {
            CS1[i].ethg++ <--> cable <--> AS1[j].ethg++;
        }

        for i=0..1, for j=0..1 {
            AS1[i].ethg++ <--> cable <--> AcS1[j].ethg++;

        }

        for i=2..3, for j=2..3 {
            AS1[i].ethg++ <--> cable <--> AcS1[j].ethg++;

        }
        for i=4..5, for j=4..5 {
            AS1[i].ethg++ <--> cable <--> AcS1[j].ethg++;

        }
        for i=6..7, for j=6..7 {
            AS1[i].ethg++ <--> cable <--> AcS1[j].ethg++;

        }
        for j=0..servercount-1 {
            AcS1[0].ethg++ <--> cable <--> ServerNode[j].ethg++;
        }

        for j=servercount..servercount*2-1 {
            AcS1[1].ethg++ <--> cable <--> ServerNode[j].ethg++;
        }

        for j=servercount*2..servercount*3-1 {
            AcS1[2].ethg++ <--> cable <--> ServerNode[j].ethg++;
        }
        for j=servercount*3..servercount*4-1 {
            AcS1[3].ethg++ <--> cable <--> ServerNode[j].ethg++;
        }
        for j=8..servercount*5-1 {
            AcS1[4].ethg++ <--> cable <--> ServerNode[j].ethg++;
        }
        for j=10..servercount*6-1 {
            AcS1[5].ethg++ <--> cable <--> ServerNode[j].ethg++;
        }
        for j=12..servercount*7-1 {
            AcS1[6].ethg++ <--> cable <--> ServerNode[j].ethg++;
        }
        for j=14..servercount*8-1 {
            AcS1[7].ethg++ <--> cable <--> ServerNode[j].ethg++;
        }

}



