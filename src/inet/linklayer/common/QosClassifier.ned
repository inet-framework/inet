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
        string ipProtocolUpMap = default("1 BE 58 BE");
        string udpPortUpMap = default("21 BK 80 BE 4000 VI 5000 VO");
        string tcpPortUpMap = default("21 BK 80 BE 4000 VI 5000 VO");
        string defaultUp = default("BE");
        @display("i=block/classifier");
    gates:
        input in;
        output out;
}

