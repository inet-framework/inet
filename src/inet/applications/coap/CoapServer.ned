//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.applications.coap;

//
// CoAP over UDP server application hosting configurable XML resources and
// serving CoAP requests per RFC 7252.
//
simple CoapServer extends CoapEndpointBase
{
    parameters:
        @class(CoapServer);
        localPort = default(5683);
        xml resources = default(xml("<resources/>"));
        double responseDelay @unit(s) = default(0s);
        double defaultMaxAge @unit(s) = default(60s);

        @signal[requestHandled](type=long);
        @signal[activeObservers](type=long);
        @signal[notificationSent](type=long);
        @statistic[requestHandled](title="server requests handled"; source=requestHandled; record=count,sum; interpolationmode=none);
        @statistic[activeObservers](title="active observers"; source=activeObservers; record=vector,last,max; interpolationmode=sample-hold);
        @statistic[notificationSent](title="notifications sent"; source=notificationSent; record=count,sum; interpolationmode=none);
}
