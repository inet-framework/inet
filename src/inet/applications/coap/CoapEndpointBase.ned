//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.applications.coap;

import inet.applications.contract.IApp;

//
// Base module for CoAP-over-UDP endpoints implementing RFC 7252 message encoding,
// deduplication, token/MID allocation, response matching, and error handling.
//
simple CoapEndpointBase like IApp
{
    parameters:
        @class(CoapEndpointBase);
        string interfaceTableModule = default("");
        string dtlsModule = default("");
        string localAddress = default("");
        int localPort = default(-1);
        int timeToLive = default(-1);
        int dscp = default(-1);
        int tos = default(-1);
        int rngStream = default(0);

        double ackTimeout @unit(s) = default(2s);
        double ackRandomFactor = default(1.5);
        int maxRetransmit = default(4);
        int nstart = default(1);
        double probingRate @unit(bps) = default(8bps);
        double maxLatency @unit(s) = default(100s);
        double processingDelay @unit(s) = default(2s);

        int maxPendingRequests = default(64);
        int maxPendingOutboundPerEndpoint = default(64);
        int maxIncomingBlockSize @unit(B) = default(65536B);
        int preferredBlockSize @unit(B) = default(1024B);
        int maxIncomingBlockTransfers = default(16);
        int maxObservers = default(64);

        double stopOperationExtraTime @unit(s) = default(-1s);
        double stopOperationTimeout @unit(s) = default(2s);

        @display("i=block/app");
        @lifecycleSupport;
        @signal[packetSent](type=inet::Packet);
        @signal[packetReceived](type=inet::Packet);
        @signal[packetSentToLower](type=inet::Packet);
        @signal[packetReceivedFromLower](type=inet::Packet);
        @signal[retransmission](type=long);
        @signal[duplicateReceived](type=long);
        @signal[malformedReceived](type=long);
        @signal[capacityRejected](type=long);

        @statistic[packetSent](title="packets sent"; source=packetSent; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[packetReceived](title="packets received"; source=packetReceived; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[retransmission](title="retransmissions"; source=retransmission; record=count,sum; interpolationmode=none);
        @statistic[duplicateReceived](title="duplicate messages received"; source=duplicateReceived; record=count,sum; interpolationmode=none);
        @statistic[malformedReceived](title="malformed messages received"; source=malformedReceived; record=count,sum; interpolationmode=none);
        @statistic[capacityRejected](title="capacity rejections"; source=capacityRejected; record=count,sum; interpolationmode=none);

    gates:
        input socketIn @labels(UdpCommand/up);
        output socketOut @labels(UdpCommand/down);
}
