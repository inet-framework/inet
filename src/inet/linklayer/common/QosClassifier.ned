//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.common;

import inet.common.SimpleModule;

//
// Classifies packets and assigns a User Priority based on the IP
// protocol or the transport protocol port numbers.
//
simple QosClassifier extends SimpleModule like IIeee8021dQosClassifier
{
    parameters:
        @class(QosClassifier);
        object ipProtocolUpMap = default([ {protocol:1, up:"BE"}, {protocol:58, up:"BE"} ]);
        object udpPortUpMap = default([ {port:21, up:"BK"}, {port:80, up:"BE"}, {port:4000, up:"VI"}, {port:5000, up:"VO"} ]);
        object tcpPortUpMap = default([ {port:21, up:"BK"}, {port:80, up:"BE"}, {port:4000, up:"VI"}, {port:5000, up:"VO"} ]);
        string defaultUp = default("BE");
        @display("i=block/classifier");
    gates:
        input in;
        output out;
}

