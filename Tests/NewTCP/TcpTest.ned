
import
    "TcpTestClient.ned",
    "TCPTester.ned",
    "TCP.ned",
    "TCPDump.ned";


module TcpTestNet1
    parameters:
        testing: bool;
    submodules:
        cli: TcpTestClient;
            display: "p=95,95;i=block/app";
        tcpcli: TCP;
            display: "p=95,178;i=block/wheelbarrow";
        tcpsrv: TCP;
            display: "p=302,178;i=block/wheelbarrow";
        srv: TcpTestClient;
            display: "p=303,96;i=block/app";
        tcptester: TCPScriptableTester;
            display: "p=198,178;i=device/router";
    connections:
        cli.tcpOut --> tcpcli.from_appl++;
        cli.tcpIn <-- tcpcli.to_appl++;
        srv.tcpOut --> tcpsrv.from_appl++;
        srv.tcpIn <-- tcpsrv.to_appl++;

        tcpcli.to_ip --> delay 1ms --> tcptester.in1;
        tcpcli.from_ip <-- delay 1ms <-- tcptester.out1;
        tcptester.out2 --> delay 1ms --> tcpsrv.from_ip;
        tcptester.in2 <-- delay 1ms <-- tcpsrv.to_ip;
endmodule


module TcpTestNet2
    parameters:
        testing: bool;
    submodules:
        cli: TcpTestClient;
            display: "p=95,95;i=block/app";
        tcpcli: TCP;
            display: "p=95,178;i=block/wheelbarrow";
        tcpsrv: TCP;
            display: "p=302,178;i=block/wheelbarrow";
        srv: TcpTestClient;
            display: "p=303,96;i=block/app";
        tcptester: TCPRandomTester;
            display: "p=198,178;i=device/router";
    connections:
        cli.tcpOut --> tcpcli.from_appl++;
        cli.tcpIn <-- tcpcli.to_appl++;
        srv.tcpOut --> tcpsrv.from_appl++;
        srv.tcpIn <-- tcpsrv.to_appl++;

        tcpcli.to_ip --> tcptester.in1;
        tcpcli.from_ip <-- tcptester.out1;
        tcptester.out2 --> delay 1ms datarate 1000000 --> tcpsrv.from_ip;
        tcptester.in2 <-- delay 1ms datarate 1000000 <-- tcpsrv.to_ip;
endmodule

network tcpTestNet1 : TcpTestNet1
endnetwork

network tcpTestNet2 : TcpTestNet2
endnetwork
