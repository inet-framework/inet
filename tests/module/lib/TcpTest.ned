package inet.test.moduletest.lib;

import inet.protocolelement.processing.DelayChannel;
import inet.transportlayer.contract.ITcp;

network TcpTestNet1
{
    parameters:
        bool testing;
    submodules:
        cli_app: TcpTestClient {
            @display("p=95,95");
        }
        cli_tcp: <default("Tcp")> like ITcp {
            @display("p=95,178");
        }
        srv_tcp: <default("Tcp")> like ITcp {
            @display("p=302,178");
        }
        srv_app: TcpTestClient {
            @display("p=303,96");
        }
        tcptester: TCPScriptableTester {
            @display("p=198,178");
        }
        d1: DelayChannel {
            *.delay = 1ms;
        }
        d2: DelayChannel {
            *.delay = 1ms;
        }
    connections allowunconnected:
        cli_app.socketOut --> cli_tcp.appIn;
        cli_app.socketIn <-- cli_tcp.appOut;
        srv_app.socketOut --> srv_tcp.appIn;
        srv_app.socketIn <-- srv_tcp.appOut;

        cli_tcp.ipOut --> d1.leftIn;
        d1.rightOut --> tcptester.in1;
        cli_tcp.ipIn <-- d1.leftOut;
        d1.rightIn <-- tcptester.out1;
        tcptester.out2 --> d2.leftIn;
        d2.rightOut --> srv_tcp.ipIn;
        tcptester.in2 <-- d2.leftOut;
        d2.rightIn <-- srv_tcp.ipOut;
}


network TcpTestNet2
{
    parameters:
        bool testing;
    submodules:
        cli_app: TcpTestClient {
            @display("p=95,95");
        }
        cli_tcp: <default(firstAvailable("Tcp", "TcpLwip", "TcpNsc"))> like ITcp {
            @display("p=95,178");
        }
        srv_tcp: <default(firstAvailable("Tcp", "TcpLwip", "TcpNsc"))> like ITcp {
            @display("p=302,178");
        }
        srv_app: TcpTestClient {
            @display("p=303,96");
        }
        tcptester: TCPRandomTester {
            @display("p=198,178");
        }
        d: DelayChannel {
            *.delay = 1ms;
        }
    connections allowunconnected:
        cli_app.socketOut --> cli_tcp.appIn;
        cli_app.socketIn <-- cli_tcp.appOut;
        srv_app.socketOut --> srv_tcp.appIn;
        srv_app.socketIn <-- srv_tcp.appOut;

        cli_tcp.ipOut --> tcptester.in1;
        cli_tcp.ipIn <-- tcptester.out1;

        tcptester.out2 --> d.leftIn;
        d.rightOut --> srv_tcp.ipIn;
        tcptester.in2 <-- d.leftOut;
        d.rightIn <-- srv_tcp.ipOut;
}

