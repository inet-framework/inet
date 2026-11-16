package inet.protocoltest.dtls;

import inet.applications.contract.IApp;
import inet.common.MessageDispatcher;
import inet.transportlayer.dtls.Dtls;

simple ProbeDtls extends Dtls
{
    @class(::inet::protocoltest::dtls::ProbeDtls);
}

module DtlsHeartbeatTestApp like IApp
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
        dtls: ProbeDtls {
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
