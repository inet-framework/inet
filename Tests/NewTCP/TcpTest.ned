
import
    "TcpTestClient.ned",
    "TCPMain.ned",
    "TCPDump.ned";


module TcpTest
    parameters:
        testing: bool;
    submodules:
        cli: TcpTestClient;
            display: "p=95,95;i=app";
        tcpcli: TCPMain;
            display: "p=95,178;i=prot2";
        tcpsrv: TCPMain;
            display: "p=302,178;i=prot2";
        srv: TcpTestClient;
            display: "p=303,96;i=app";
        tcptester: TCPTester;
            display: "p=198,178;i=ipc";
    connections:
        cli.tcpOut --> tcpcli.from_appl;
        cli.tcpIn <-- tcpcli.to_appl;
        srv.tcpOut --> tcpsrv.from_appl;
        srv.tcpIn <-- tcpsrv.to_appl;

        tcpcli.to_ip --> delay 1ms --> tcptester.in1;
        tcpcli.from_ip <-- delay 1ms <-- tcptester.out1;
        tcptester.out2 --> delay 1ms --> tcpsrv.from_ip;
        tcptester.in2 <-- delay 1ms <-- tcpsrv.to_ip;
endmodule


network tcpTest : TcpTest
endnetwork
