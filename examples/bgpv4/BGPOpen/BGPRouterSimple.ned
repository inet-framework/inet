//
package inet.examples.bgpv4.BGPOpen;

import inet.base.NotificationBoard;
import inet.networklayer.common.InterfaceTable;
import inet.networklayer.ipv4.RoutingTable;
import inet.nodes.inet.NetworkLayer;
import inet.transport.tcp.TCP;
import inet.networklayer.bgpv4.BGPRouting;
import inet.linklayer.ppp.PPPInterfaceNoQueue;
import inet.base.MessageChecker;

//
// \IP router.
//
module BGPRouterSimple
{
    parameters:
        string routingFile;
    gates:
        input pppIn[];
        output pppOut[];
    submodules:
        notificationBoard: NotificationBoard {
            parameters:
                @display("p=52,44;i=block/control_s");
        }
        interfaceTable: InterfaceTable {
            parameters:
                @display("p=52,108;i=block/table_s");
        }
        routingTable: RoutingTable {
            parameters:
                IPForward = true;
                routerId = "auto";
                routingFile = routingFile;
                @display("p=52,172;i=block/table2_s");
        }
        bgp: BGPRouting {
            parameters:
                @display("p=160,50;i=block/network");
        }
        snifferIn: MessageChecker {
            parameters:
                @display("i=block/uparrow_s;p=132,124");
        }
        snifferOut: MessageChecker {
            parameters:
                @display("i=block/downarrow_s;p=196,124");
        }
        tcp: TCP {
            parameters:
                @display("p=160,202;i=block/transport");
        }        
        networkLayer: NetworkLayer {
            parameters:
                proxyARP = true;
                @display("p=120,284;i=block/layer;q=queue");
            gates:
                ifIn[sizeof(pppOut)];
                ifOut[sizeof(pppOut)];
        }
        ppp[sizeof(pppOut)]: PPPInterfaceNoQueue {
            parameters:
                @display("p=74,369,row,110;q=l2queue;i=block/ifcard");
        }
    connections allowunconnected:
        bgp.toTCP --> { @display("m=s"); } --> snifferOut.in;
        bgp.fromTCP <-- { @display("m=s"); } <-- snifferIn.out;

        snifferOut.out --> { @display("m=s"); } --> tcp.appIn++;
        snifferIn.in <-- { @display("m=s"); } <-- tcp.appOut++;

        tcp.ipOut --> { @display("m=s"); } --> networkLayer.tcpIn;
        tcp.ipIn <-- { @display("m=s"); } <-- networkLayer.tcpOut;

        // connections to network outside		
        for i=0..sizeof(pppOut)-1 {
            pppIn[i] --> { @display("m=s"); } --> ppp[i].phys$i;
            pppOut[i] <-- { @display("m=s"); } <-- ppp[i].phys$o;
            ppp[i].netwOut --> networkLayer.ifIn[i];
            ppp[i].netwIn <-- networkLayer.ifOut[i];
        }
}

