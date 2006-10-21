import
    "NotificationBoard",
    "InterfaceTable",
    "RoutingTable",
    "PPPInterface",
    "NetworkLayer",
    "TCP",
    "UDP",
    "TestNop";

module TestRouter
    parameters:
        routingFile: string,
        routerId: string,
        testUnit: string;
    gates:
        in: in[];
        out: out[];
    submodules:
        notificationBoard: NotificationBoard;
            display: "p=60,60;i=block/control";
        interfaceTable: InterfaceTable;
            display: "p=150,60;i=block/table";
        routingTable: RoutingTable;
            parameters:
                IPForward = true,
                routerId = routerId,
                routingFile = routingFile;
            display: "p=240,60;i=block/table";
        networkLayer: NetworkLayer;
            gatesizes:
                ifIn[sizeof(out)],
                ifOut[sizeof(out)];
            display: "p=200,341;i=block/fork;q=queue";
        ppp: PPPInterface[sizeof(out)];
            display: "p=90,457,row,110;q=l2queue;i=block/ifcard";
        tcp: TCP;
            display: "p=200,250;i=block/wheelbarrow";
        udp: UDP;
            display: "p=300,250;i=block/wheelbarrow";
        unit: testUnit like TestNop;
            parameters:
            display: "p=250, 150;i=block/cogwheel";
    connections nocheck:
        // connections to network outside
        for i=0..sizeof(out)-1 do
            in[i] --> ppp[i].physIn;
            out[i] <-- ppp[i].physOut;
            ppp[i].netwOut --> networkLayer.ifIn[i];
            ppp[i].netwIn <-- networkLayer.ifOut[i];
        endfor;

        tcp.to_ip --> networkLayer.TCPIn;
        tcp.from_ip <-- networkLayer.TCPOut;

        udp.to_ip --> networkLayer.UDPIn;
        udp.from_ip <-- networkLayer.UDPOut;

        unit.tcpIn <-- tcp.to_appl++;
        unit.tcpOut --> tcp.from_appl++;

        unit.udpIn <-- udp.to_app++;
        unit.udpOut --> udp.from_app++;

        unit.ipOut --> networkLayer.OSPFIn;
        unit.ipIn <-- networkLayer.OSPFOut;
endmodule

