//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.contract;

//
// This module interface is implemented by packet classifier modules. A packet
// classifier is a passive module which has one passive input and multiple
// active outputs. Packets pushed into the passive input are passed through to
// one of the active outputs without any delay and reordering.
//
moduleinterface IPacketClassifier extends IPassivePacketSink
{
    parameters:
        @display("i=block/classifier");
    gates:
        output out[] @labels(push);
}
