//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.protocoltest.coap;

import inet.applications.contract.IApp;

//
// Raw UDP test peer for injecting valid and malformed CoAP datagrams
// and recording received wire datagrams independently of inet.applications.coap.
//
simple CoapTestPeer like IApp
{
    parameters:
        @class(CoapTestPeer);
        string localAddress = default("");
        int localPort = default(5683);
        double stopOperationExtraTime @unit(s) = default(-1s);
        double stopOperationTimeout @unit(s) = default(2s);

        @display("i=block/app");
        @lifecycleSupport;
        @signal[packetSent](type=inet::Packet);
        @signal[packetReceived](type=inet::Packet);
        @signal[packetSentToLower](type=inet::Packet);
        @signal[packetReceivedFromLower](type=inet::Packet);

    gates:
        input socketIn @labels(UdpCommand/up);
        output socketOut @labels(UdpCommand/down);
}
