
import
    "TcpTestClient.ned",
    "TCPMain.ned",
    "TCPDump.ned";


module TcpTest
    submodules:
        cli: TcpTestClient;
            display: "p=95,95;i=app";
        tcpcli: TCPMain;
            display: "p=95,178;i=prot2";
        tcpsrv: TCPMain;
            display: "p=302,178;i=prot2";
        srv: TcpTestClient;
            display: "p=303,96;i=app";
        tcpdump: TCPDump;
            display: "p=240,178;i=ipc";
    connections:
        cli.tcpOut --> tcpcli.from_appl;
        cli.tcpIn <-- tcpcli.to_appl;
        srv.tcpOut --> tcpsrv.from_appl;
        srv.tcpIn <-- tcpsrv.to_appl;

        tcpcli.to_ip --> delay 0.1 datarate 1000000 --> tcpdump.in1;
        tcpcli.from_ip <-- delay 0.1 datarate 1000000 <-- tcpdump.out1;
        tcpdump.out2 --> tcpsrv.from_ip;
        tcpdump.in2 <-- tcpsrv.to_ip;
endmodule


network tcpTest : TcpTest
endnetwork
