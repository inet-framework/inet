// package module_.lib;

import inet.transportlayer.ITCP;


network TcpTestNet1
{
    parameters:
        bool testing;
        string tcpType = default(firstAvailable("TCP", "TCP_lwIP", "TCP_NSC", "TCP_None"));  // tcp implementation (e.g. ~TCP, ~TCP_lwIP, ~TCP_NSC) or ~TCPSpoof
    submodules:
        cli_app: TcpTestClient {
            @display("p=95,95");
        }
        cli_tcp: <tcpType> like ITCP {
            @display("p=95,178");
        }
        srv_tcp: <tcpType> like ITCP {
            @display("p=302,178");
        }
        srv_app: TcpTestClient {
            @display("p=303,96");
        }
        tcptester: TCPScriptableTester {
            @display("p=198,178");
        }
    connections allowunconnected:
        cli_app.tcpOut --> cli_tcp.appIn++;
        cli_app.tcpIn <-- cli_tcp.appOut++;
        srv_app.tcpOut --> srv_tcp.appIn++;
        srv_app.tcpIn <-- srv_tcp.appOut++;

        cli_tcp.ipOut --> {  delay = 1ms; } --> tcptester.in1;
        cli_tcp.ipIn <-- {  delay = 1ms; } <-- tcptester.out1;
        tcptester.out2 --> {  delay = 1ms; } --> srv_tcp.ipIn;
        tcptester.in2 <-- {  delay = 1ms; } <-- srv_tcp.ipOut;
}


network TcpTestNet2
{
    parameters:
        bool testing;
        string tcpType = default(firstAvailable("TCP", "TCP_lwIP", "TCP_NSC", "TCP_None"));  // tcp implementation (e.g. ~TCP, ~TCP_lwIP, ~TCP_NSC) or ~TCPSpoof
    submodules:
        cli_app: TcpTestClient {
            @display("p=95,95");
        }
        cli_tcp: <tcpType> like ITCP {
            @display("p=95,178");
        }
        srv_tcp: <tcpType> like ITCP {
            @display("p=302,178");
        }
        srv_app: TcpTestClient {
            @display("p=303,96");
        }
        tcptester: TCPRandomTester {
            @display("p=198,178");
        }
    connections allowunconnected:
        cli_app.tcpOut --> cli_tcp.appIn++;
        cli_app.tcpIn <-- cli_tcp.appOut++;
        srv_app.tcpOut --> srv_tcp.appIn++;
        srv_app.tcpIn <-- srv_tcp.appOut++;

        cli_tcp.ipOut --> tcptester.in1;
        cli_tcp.ipIn <-- tcptester.out1;
        tcptester.out2 --> {  delay = 1ms; } --> srv_tcp.ipIn;
        tcptester.in2 <-- {  delay = 1ms; } <-- srv_tcp.ipOut;
}

