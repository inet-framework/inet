//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.protocoltest.coapdtls;

import inet.applications.contract.IApp;
import inet.transportlayer.contract.IDtls;

//
// Raw DTLS application-layer peer for injecting and inspecting arbitrary CoAP byte
// payloads over real DTLS 1.3 associations inside CoAP-over-DTLS protocol tests.
//
simple CoapDtlsTestPeer
{
    parameters:
        @class(CoapDtlsTestPeer);
        string dtlsModule = default("^.dtls");
        bool server = default(false);
        double stopOperationExtraTime @unit(s) = default(-1s);
        double stopOperationTimeout @unit(s) = default(2s);

        @display("i=block/app");
        @lifecycleSupport;
        @signal[packetSent](type=inet::Packet);
        @signal[packetReceived](type=inet::Packet);
        @signal[packetSentToLower](type=inet::Packet);
        @signal[packetReceivedFromLower](type=inet::Packet);
        @signal[controlStateChanged](type=long);
}

//
// Compound test peer wrapping Dtls + CoapDtlsTestPeer behind IApp.
//
module CoapDtlsTestApp like IApp
{
    parameters:
        bool server = default(false);
        @application;
        @display("i=block/app");
    gates:
        input socketIn;
        output socketOut;
    submodules:
        dtls: <default("Dtls")> like IDtls {
            localPort = default(parent.server ? 5684 : -1);
            @display("p=200,280");
        }
        peer: CoapDtlsTestPeer {
            dtlsModule = "^.dtls";
            server = default(parent.server);
            @display("p=200,150");
        }
    connections allowunconnected:
        socketIn --> dtls.socketIn;
        dtls.socketOut --> socketOut;
}
