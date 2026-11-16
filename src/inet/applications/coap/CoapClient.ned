//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.applications.coap;

//
// CoAP over UDP client application (RFC 7252) issuing GET/POST/PUT/DELETE
// requests to a peer endpoint with configurable timing and options.
//
simple CoapClient extends CoapEndpointBase
{
    parameters:
        @class(CoapClient);
        string peerAddress = default("");
        int peerPort = default(5683);
        string requestUri = default("/");
        string requestMethod @enum("GET","POST","PUT","DELETE") = default("GET");
        bool confirmable = default(true);
        string requestBody = default("");
        int contentFormat = default(-1);
        int accept = default(-1);
        bool observe = default(false);
        bool useRequestTag = default(false);

        double startTime @unit(s) = default(1s);
        double stopTime @unit(s) = default(-1s);
        volatile double sendInterval @unit(s) = default(0s);
        int numRequests = default(1);

        @signal[requestStarted](type=long);
        @signal[requestCompleted](type=long);
        @signal[requestFailed](type=long);
        @signal[requestAborted](type=long);
        @signal[requestLatency](type=simtime_t);

        @statistic[requestStarted](title="requests started"; source=requestStarted; record=count,sum; interpolationmode=none);
        @statistic[requestCompleted](title="requests completed"; source=requestCompleted; record=count,sum; interpolationmode=none);
        @statistic[requestFailed](title="requests failed"; source=requestFailed; record=count,sum; interpolationmode=none);
        @statistic[requestAborted](title="requests aborted"; source=requestAborted; record=count,sum; interpolationmode=none);
        @statistic[requestLatency](title="request latency"; source=requestLatency; unit=s; record=stats,vector; interpolationmode=none);
}
