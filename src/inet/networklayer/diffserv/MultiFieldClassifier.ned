//
// Copyright (C) 2012 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.networklayer.diffserv;

import inet.queueing.base.PacketClassifierBase;

//
// This classifier contains a list of filters that identifies
// the flows and determines their classes.
// Each filter can match the source and destination address,
// IP protocol number, source and destination ports, or ToS
// of the datagram. The first matching filter determines the
// index of the out gate. If no matching filter is found,
// then the packet will be sent through the defaultOut gate.
//
// See RFC 2475 2.3.1, RFC 3290 4.2.2
//
simple MultiFieldClassifier extends PacketClassifierBase
{
    parameters:
        xml filters = default(xml("<filters/>"));
        @class(MultiFieldClassifier);
        @signal[pkClass](type=long);
        @statistic[pkClass](title="packet class"; source=pkClass; record=vector; interpolationmode=none);
    gates:
        output defaultOut;
}

