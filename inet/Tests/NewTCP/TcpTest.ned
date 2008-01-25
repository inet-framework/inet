


network TcpTestNet1
{
    parameters:
        bool testing;
    submodules:
        cli: TcpTestClient {
            parameters:
                @display("p=95,95");
        };
        tcpcli: TCP {
            parameters:
                @display("p=95,178");
        };
        tcpsrv: TCP {
            parameters:
                @display("p=302,178");
        };
        srv: TcpTestClient {
            parameters:
                @display("p=303,96");
        };
        tcptester: TCPScriptableTester {
            parameters:
                @display("p=198,178");
        };
    connections:
        cli.tcpOut --> tcpcli.from_appl++;
        cli.tcpIn <-- tcpcli.to_appl++;
        srv.tcpOut --> tcpsrv.from_appl++;
        srv.tcpIn <-- tcpsrv.to_appl++;

        tcpcli.to_ip --> {  delay = 1ms; } --> tcptester.in1;
        tcpcli.from_ip <-- {  delay = 1ms; } <-- tcptester.out1;
        tcptester.out2 --> {  delay = 1ms; } --> tcpsrv.from_ip;
        tcptester.in2 <-- {  delay = 1ms; } <-- tcpsrv.to_ip;
}


network TcpTestNet2
{
    parameters:
        bool testing;
    submodules:
        cli: TcpTestClient {
            parameters:
                @display("p=95,95");
        };
        tcpcli: TCP {
            parameters:
                @display("p=95,178");
        };
        tcpsrv: TCP {
            parameters:
                @display("p=302,178");
        };
        srv: TcpTestClient {
            parameters:
                @display("p=303,96");
        };
        tcptester: TCPRandomTester {
            parameters:
                @display("p=198,178");
        };
    connections:
        cli.tcpOut --> tcpcli.from_appl++;
        cli.tcpIn <-- tcpcli.to_appl++;
        srv.tcpOut --> tcpsrv.from_appl++;
        srv.tcpIn <-- tcpsrv.to_appl++;

        tcpcli.to_ip --> tcptester.in1;
        tcpcli.from_ip <-- tcptester.out1;
        tcptester.out2 --> {  delay = 1ms; datarate = 1000000; } --> tcpsrv.from_ip;
        tcptester.in2 <-- {  delay = 1ms; datarate = 1000000; } <-- tcpsrv.to_ip;
}

