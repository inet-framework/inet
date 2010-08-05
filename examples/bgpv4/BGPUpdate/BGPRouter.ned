//

package inet.examples.bgpv4.BGPUpdate;

import inet.base.NotificationBoard;
import inet.linklayer.ethernet.EthernetInterface;
import inet.networklayer.common.InterfaceTable;
import inet.networklayer.ipv4.RoutingTable;
import inet.nodes.inet.NetworkLayer;
import inet.transport.tcp.TCP;
import inet.networklayer.bgpv4.BGPRouting;
import inet.linklayer.ppp.PPPInterfaceNoQueue;
import inet.base.MessageChecker;
import inet.networklayer.ospfv2.OSPFRouting;

//
// \IP router.
//
module BGPRouter
{
    parameters:
        string routingFile = default("");
    gates:
        inout pppg[] @labels(PPPFrame-conn);
        inout ethg[] @labels(EtherFrame-conn);
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
        ospf: OSPFRouting {
            parameters:
                @display("p=252,188,row;i=block/network2");
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
                ifIn[sizeof(pppg)+sizeof(ethg)];
                ifOut[sizeof(pppg)+sizeof(ethg)];
        }
        ppp[sizeof(pppg)]: PPPInterfaceNoQueue {
            parameters:
                @display("p=74,369,row,110;q=l2queue;i=block/ifcard");
        }
        eth[sizeof(ethg)]: EthernetInterface {
            parameters:
                @display("p=97,257,row,110;q=l2queue;i=block/ifcard");
        }
    connections allowunconnected:
        bgp.toTCP --> { @display("m=s"); } --> snifferOut.in;
        bgp.fromTCP <-- { @display("m=s"); } <-- snifferIn.out;

        snifferOut.out --> { @display("m=s"); } --> tcp.appIn++;
        snifferIn.in <-- { @display("m=s"); } <-- tcp.appOut++;

        tcp.ipOut --> { @display("m=s"); } --> networkLayer.tcpIn;
        tcp.ipIn <-- { @display("m=s"); } <-- networkLayer.tcpOut;

        ospf.ipOut --> { @display("m=s"); } --> networkLayer.ospfIn;
        ospf.ipIn <-- { @display("m=s"); } <-- networkLayer.ospfOut;

        // connections to network outside
        for i=0..sizeof(pppg)-1 {
            pppg[i] <--> ppp[i].phys;
            ppp[i].netwOut --> networkLayer.ifIn[i];
            ppp[i].netwIn <-- networkLayer.ifOut[i];
        }

        for i=0..sizeof(ethg)-1 {
            ethg[i] <--> eth[i].phys;
            eth[i].netwOut --> networkLayer.ifIn[sizeof(pppg)+i];
            eth[i].netwIn <-- networkLayer.ifOut[sizeof(pppg)+i];
        }		
}

