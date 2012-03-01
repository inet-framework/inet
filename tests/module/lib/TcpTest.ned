// package module_.lib;

import inet.transport.tcp.TCP;


network TcpTestNet1
{
    parameters:
        bool testing;
    submodules:
        cli: TcpTestClient {
            @display("p=95,95");
        }
        tcpcli: TCP {
            @display("p=95,178");
        }
        tcpsrv: TCP {
            @display("p=302,178");
        }
        srv: TcpTestClient {
            @display("p=303,96");
        }
        tcptester: TCPScriptableTester {
            @display("p=198,178");
        }
    connections:
        cli.tcpOut --> tcpcli.appIn++;
        cli.tcpIn <-- tcpcli.appOut++;
        srv.tcpOut --> tcpsrv.appIn++;
        srv.tcpIn <-- tcpsrv.appOut++;

        tcpcli.ipOut --> {  delay = 1ms; } --> tcptester.in1;
        tcpcli.ipIn <-- {  delay = 1ms; } <-- tcptester.out1;
        tcptester.out2 --> {  delay = 1ms; } --> tcpsrv.ipIn;
        tcptester.in2 <-- {  delay = 1ms; } <-- tcpsrv.ipOut;
}


network TcpTestNet2
{
    parameters:
        bool testing;
    submodules:
        cli: TcpTestClient {
            @display("p=95,95");
        }
        tcpcli: TCP {
            @display("p=95,178");
        }
        tcpsrv: TCP {
            @display("p=302,178");
        }
        srv: TcpTestClient {
            @display("p=303,96");
        }
        tcptester: TCPRandomTester {
            @display("p=198,178");
        }
    connections:
        cli.tcpOut --> tcpcli.appIn++;
        cli.tcpIn <-- tcpcli.appOut++;
        srv.tcpOut --> tcpsrv.appIn++;
        srv.tcpIn <-- tcpsrv.appOut++;

        tcpcli.ipOut --> tcptester.in1;
        tcpcli.ipIn <-- tcptester.out1;
        tcptester.out2 --> {  delay = 1ms; datarate = 1Mbps; } --> tcpsrv.ipIn;
        tcptester.in2 <-- {  delay = 1ms; datarate = 1Mbps; } <-- tcpsrv.ipOut;
}


