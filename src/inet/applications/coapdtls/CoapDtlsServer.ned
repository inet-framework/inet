//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.applications.coapdtls;

import inet.applications.coap.CoapServer;
import inet.applications.contract.IApp;
import inet.transportlayer.contract.IDtls;

//
// Compound CoAP-over-DTLS 1.3 server application (RFC 7252 Section 9, coaps://, UDP port 5684).
// Declares dtls before coap so DTLS is OPERATING when coap binds its listener at startup.
//
module CoapDtlsServer like IApp
{
    parameters:
        @application;
        @display("i=block/app");
    gates:
        input socketIn;
        output socketOut;
    submodules:
        dtls: <default("Dtls")> like IDtls {
            localPort = default(5684);
            @display("p=200,280");
        }
        coap: CoapServer {
            dtlsModule = "^.dtls";
            @display("p=200,150");
        }
    connections allowunconnected:
        socketIn --> dtls.socketIn;
        dtls.socketOut --> socketOut;
}
