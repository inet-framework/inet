//
// Copyright (C) 2004 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//


package inet.examples.ngoa.ecr;

import inet.examples.ethernet.lans.ethernetline;
import inet.networklayer.autorouting.FlatNetworkConfigurator;
import inet.nodes.inet.Router;
import inet.nodes.inet.StandardHost;
import inet.world.ChannelInstaller;
import inet.world.NAMTrace;
import ned.DatarateChannel;


network ECRTest
{
    parameters:
        int n;
    types:
        channel fiberline extends DatarateChannel
        {
            delay = 1us;
            datarate = 512Mbps;
        }
        channel fiberline_100M extends DatarateChannel
        {
            delay = 0.1us;
            datarate = 100Mbps;
        }
        channel fiberline_10G extends DatarateChannel
        {
            delay = 1us;
            datarate = 10Gbps;
        }
        channel fiberline_10G_10ms_rtt extends DatarateChannel
        {
            delay = 5ms;
            datarate = 10Gbps;
        }
        channel fiberline_10G_20ms_rtt extends DatarateChannel
        {
            delay = 10ms;
            datarate = 10Gbps;
        }
        channel fiberline_1T_10ms_rtt extends DatarateChannel
        {
            delay = 5ms;
            datarate = 1Tbps;
        }
        channel fiberline_1T_20ms_rtt extends DatarateChannel
        {
            delay = 10ms;
            datarate = 1Tbps;
        }         
//        channel ethernetline extends DatarateChannel
//        {
//            delay = 0.1us;
//            datarate = 10Mbps;
//        }
//        channel ethernetline_1M extends DatarateChannel
//        {
//            delay = 0.1us;
//            datarate = 1Mbps;
//        }
//         channel ethernetline_31M extends DatarateChannel
//        {
//            delay = 0.1us;
//            datarate = 31Mbps;
//        }
    submodules:
        channelInstaller: ChannelInstaller {
            parameters:
                channelClass = "ThruputMeteringChannel";
                channelAttrs = "format=u";
                @display("p=98,50");
        }
        configurator: FlatNetworkConfigurator {
            parameters:
                @display("p=185,50");
        }
//        nam: NAMTrace {
//            parameters:
//                @display("p=260,50");
//        }
        r1: Router {
            parameters:
                @display("p=185,194");
        }
        r2: Router {
            parameters:
                @display("p=304,194");
        }
        cli[n]: StandardHost {
            parameters:
                @display("i=device/laptop;p=67,194");
        }
        srv: StandardHost {
            parameters:
                @display("p=439,194;i=device/server_l");
        }
    connections:
        for i=0..n-1 {
            cli[i].pppg++ <--> fiberline_100M <--> r1.pppg++;
        }
        r1.pppg++ <--> fiberline_100M <--> r2.pppg++;
        r2.pppg++ <--> fiberline_1T_10ms_rtt <--> srv.pppg++;
}
