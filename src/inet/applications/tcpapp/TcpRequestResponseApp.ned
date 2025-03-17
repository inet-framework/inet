//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.applications.tcpapp;

//
// Implements an intelligent TCP server application that generates customized responses
// based on the content of incoming requests. Extends TcpServerConnection to provide
// content-aware traffic generation capabilities, where the response pattern is determined
// by examining the request data. The first byte of each request packet is used to
// classify the request and trigger the appropriate response pattern.
//
// Designed for modeling complex server behaviors where different client requests
// require different types of responses. The application uses a RequestConsumer to
// process incoming packets and a ResponseProducer to generate appropriate response
// traffic. The response generation can be configured to create various traffic patterns
// with different packet sizes, timing distributions, and data content, enabling the
// simulation of sophisticated application-level protocols.
//
// @see ~TcpServerConnection, ~RequestConsumer, ~ResponseProducer, ~TcpServerApp
//
module TcpRequestResponseApp extends TcpServerConnection
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
