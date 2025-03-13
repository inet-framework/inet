//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.common;

//
// Implements an IEEE 802.1d QoS classifier that assigns User Priority values
// to packets based on protocol-specific information. This classifier examines
// packet headers and applies configurable mapping rules to determine the
// appropriate QoS treatment.
//
// The classifier supports three types of classification criteria:
// - IP protocol type: Maps IP protocol numbers to User Priorities
// - UDP port numbers: Maps UDP destination ports to User Priorities
// - TCP port numbers: Maps TCP destination ports to User Priorities
//
// The mapping parameters use a simple format where each pair of values represents
// a mapping rule. For example, in "80 BE 4000 VI", port 80 maps to Best Effort (BE)
// priority and port 4000 maps to Video (VI) priority.
//
// User Priority values correspond to the following traffic classes:
// - BK: Background (lowest priority)
// - BE: Best Effort (default priority)
// - VI: Video (higher priority)
// - VO: Voice (highest priority)
//
// @see ~IIeee8021dQosClassifier
//
simple QosClassifier like IIeee8021dQosClassifier
{
    parameters:
        string ipProtocolUpMap = default("1 BE 58 BE");
        string udpPortUpMap = default("21 BK 80 BE 4000 VI 5000 VO");
        string tcpPortUpMap = default("21 BK 80 BE 4000 VI 5000 VO");
        string defaultUp = default("BE");
        @display("i=block/classifier");
    gates:
        input in;
        output out;
}
