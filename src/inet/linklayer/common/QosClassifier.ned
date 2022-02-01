//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.common;

//
// This module classifies packets and assigns a User Priority based on the IP
// protocol or the transport protocol port numbers.
//
simple QosClassifier like IIeee8021dQosClassifier
{
    parameters:
        string ipProtocolUpMap = default("1 BE 58 BE");
        string udpPortUpMap @enum() = default("21 BK 80 BE 4000 VI 5000 VO");
        string tcpPortUpMap @enum() = default("21 BK 80 BE 4000 VI 5000 VO");
        string defaultUp = default("BE");
        @display("i=block/classifier");
    gates:
        input in;
        output out;
}

