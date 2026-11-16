//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.applications.coapdtls;

import inet.applications.coap.CoapClient;
import inet.applications.contract.IApp;
import inet.transportlayer.contract.IDtls;

//
// Compound CoAP-over-DTLS 1.3 client application (RFC 7252 Section 9, coaps://).
// Declares coap before dtls so node stop and crash abort CoAP state before DTLS tears down.
//
module CoapDtlsClient like IApp
{
    parameters:
        @application;
        @display("i=block/app");
    gates:
        input socketIn;
        output socketOut;
    submodules:
        coap: CoapClient {
            dtlsModule = "^.dtls";
            @display("p=200,150");
        }
        dtls: <default("Dtls")> like IDtls {
            @display("p=200,280");
        }
    connections allowunconnected:
        socketIn --> dtls.socketIn;
        dtls.socketOut --> socketOut;
}
