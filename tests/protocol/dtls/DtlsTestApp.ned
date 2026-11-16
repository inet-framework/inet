package inet.protocoltest.dtls;

import inet.applications.contract.IApp;
import inet.common.MessageDispatcher;
import inet.transportlayer.dtls.Dtls;

simple DtlsTestDriver
{
    parameters:
        @class(DtlsTestDriver);
        bool active = default(true);
        bool server = default(false);
        string peerAddress = default("host2");
        int peerPort = default(4433);
        int rawPort = default(14433);
        double startTime @unit(s) = default(0.1s);
        @signal[controlStateChanged](type=long);
        @signal[packetSentToUpper](type=inet::Packet);
    gates:
        input socketIn;
        output socketOut;
}

module DtlsTestApp like IApp
{
    parameters:
        bool server = default(false);
        bool active = default(true);
        string peerAddress = default("host2");
        int localPort = default(server ? 4433 : 4434);
    gates:
        input socketIn;
        output socketOut;
    submodules:
        dtls: Dtls {
            parameters:
                localPort = default(parent.localPort);
        }
        driver: DtlsTestDriver {
            parameters:
                server = default(parent.server);
                active = default(parent.active);
                peerAddress = default(parent.peerAddress);
                rawPort = default(parent.localPort + 10000);
        }
        dispatch: MessageDispatcher {
            parameters:
                interfaceTableModule = "^.^.interfaceTable";
        }
    connections:
        dtls.socketOut --> dispatch.in++;
        dtls.socketIn <-- dispatch.out++;
        driver.socketOut --> dispatch.in++;
        driver.socketIn <-- dispatch.out++;
        dispatch.out++ --> socketOut;
        socketIn --> dispatch.in++;
}
