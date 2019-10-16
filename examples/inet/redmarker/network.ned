//
// author: Marcel Marek
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

package inet.examples.inet.redmarker;


import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
//import inet.node.ethernet.EtherSwitch;
import inet.node.inet.Router;
//import inet.node.dmpr.DmprRouter;
//import inet.nodes.inet.Router;
//import inet.node.ospfv2.OspfRouter;
import inet.node.inet.StandardHost;
//import inet.nodes.inet.StandardHost;
import ned.DatarateChannel;

//import inet.linklayer.queue.REDDropper;

//import inet.queueing.IPacketQueue;
//import inet.linklayer.queue.DropTailQueue;
//import inet.queueing.FifoQueue;
//import inet.queueing.RedDropper;
//import inet.queueing.RedMarkerQueue;



//import inet.visualizer.integrated.IntegratedCanvasVisualizer;


channel NormalP extends DatarateChannel
{
    parameters:
        datarate = default(10Mbps);
        delay = default(1ms);
        per = 0;
        ber = 0;
}

channel FastP extends DatarateChannel
{
    parameters:
        datarate = 100Mbps;
        delay = 1ms;
        per = 0;
        ber = 0;
}




network RedMarkerNetwork
{
    parameters:
        @display("bgb=4166.738,3051.564,white;bgl=8");

    submodules:


        routerCore1: Router {
            parameters:
                @display("p=1026.4801,995.93005;i=abstract/router");

        }
        routerCore2: Router {
            parameters:
                @display("p=2945.02,995.93005;i=abstract/router");
        }
        
        configurator: Ipv4NetworkConfigurator {
//            parameters:
//                @display("p=150,115;is=m");
//                addStaticRoutes = false;
//                config = default(xml("<config>"
//                        + "<interface among='routerCore1 routerCore2' address='10.1.2.x' netmask='255.255.255.x' />"
//                        + "<interface among='routerCore1 routerCore5' address='10.1.5.x' netmask='255.255.255.x' />"
//                        + "<interface among='routerCore3 routerCore4' address='10.3.4.x' netmask='255.255.255.x' />"
//                        + "<interface among='routerCore3 routerCore5' address='10.3.5.x' netmask='255.255.255.x' />"
//                        + "<interface among='routerCore5 routerCore6' address='10.5.6.x' netmask='255.255.255.x' />"
//                        + "<interface among='routerCore2 routerCore6' address='10.2.6.x' netmask='255.255.255.x' />"
//                        + "<interface among='routerCore4 routerCore6' address='10.4.6.x' netmask='255.255.255.x' />"
//                        
//                        + "<interface among='routerCore1 sender1*' address='192.168.1.x' netmask='255.255.255.x' />"
//                        + "<interface among='routerCore2 receiver1*' address='192.168.2.x' netmask='255.255.255.x' />"
//                        + "<interface among='routerCore3 sender2*' address='192.168.3.x' netmask='255.255.255.x' />"
//                        + "<interface among='routerCore4 receiver2*' address='192.168.4.x' netmask='255.255.255.x' />"
//                        
//                        + "<interface host='sender1*' address='192.168.1.x' netmask='255.255.255.x' />"
//                        + "<interface host='receiver1*' address='192.168.2.x' netmask='255.255.255.x' />"
//                        + "<interface host='sender2*' address='192.168.3.x' netmask='255.255.255.x' />"
//                        + "<interface host='receiver2*' address='192.168.4.x' netmask='255.255.255.x' />"
//                        
//                        + "<multicast-group hosts='r*' address='224.0.0.5 224.0.0.6' />"
//                        + "<route hosts='sender1*' destination='*' gateway='routerCore1'/>"
//                        + "<route hosts='receiver1*' destination='*' gateway='routerCore2'/>"
//                        + "<route hosts='sender2*' destination='*' gateway='routerCore3'/>"
//                        + "<route hosts='receiver2*' destination='*' gateway='routerCore4'/>"
//                        + "</config>"));
//
        }


        sender1: StandardHost {
            parameters:
                @display("p=342.16,995.93005");
        }

        receiver1: StandardHost {
            parameters:
                @display("p=3708.7703,995.93005");
        }

    connections:

        sender1.ethg++ <--> FastP <--> routerCore1.ethg++;
        receiver1.ethg++ <--> FastP <--> routerCore2.ethg++;
        routerCore1.ethg++ <--> NormalP <--> routerCore2.ethg++;


}




