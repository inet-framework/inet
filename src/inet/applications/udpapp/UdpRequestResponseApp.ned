//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.applications.udpapp;

//
// This module is a generic request/response based server application. For each
// request it receives, it generates a different traffic based on the data the
// request contains. The client application can be any source that is capable
// of generating packets with different data. The first byte of the packet data
// determines the response traffic, which can be configured to produce complex
// streams of packets with various data and timing distributions.
//
module UdpRequestResponseApp extends UdpApp
{
    parameters:
        int numKind = default(2);
        sink.typename = default("RequestConsumer");
        sink.numKind = this.numKind;
        sink.responseProducerModule = default("^.source");
        sink.classifier.classifierClass = default("inet::queueing::PacketDataClassifier");
        source.typename = default("ResponseProducer");
        source.numKind = this.numKind;
}
