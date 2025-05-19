//
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "QuicZeroRttClient.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/common/packet/chunk/ByteCountChunk.h"

namespace inet {

Define_Module(QuicZeroRttClient);

void QuicZeroRttClient::handleStartOperation(LifecycleOperation *operation)
{
    //EV_DEBUG << "initialize QUIC Client stage " << stage << endl;

    L3Address localAddress = L3AddressResolver().resolve(par("localAddress"));
    int localPort = par("localPort");
    L3Address connectAddress = L3AddressResolver().resolve(par("connectAddress"));
    int connectPort = par("connectPort");

    socket1.setOutputGate(gate("socketOut"));
    socket1.bind(localAddress, localPort);
    socket1.connect(connectAddress, connectPort);
    socket1.setCallback(this);
    EV_INFO << "QuicClient::initialized with localPort=" << localPort << ", connectPort=" << connectPort << endl;

}

void QuicZeroRttClient::handleStopOperation(LifecycleOperation *operation)
{
    socket1.close();
}

void QuicZeroRttClient::handleCrashOperation(LifecycleOperation *operation)
{
}

void QuicZeroRttClient::handleMessageWhenUp(cMessage *msg)
{
    EV_DEBUG << "handle message of kind " << msg->getKind() << endl;
    if (msg->isSelfMessage()) {
        EV_DEBUG << "SelfMessage received" << endl;
    } else {
        if (socket1.belongsToSocket(msg)) {
            socket1.processMessage(msg);
        } else if (socket2.belongsToSocket(msg)) {
            socket2.processMessage(msg);
        } else {
            throw cRuntimeError("QuicZeroRttClient::handleMessageWhenUp: Got message that belongs to no socket.");
        }
    }
}

void QuicZeroRttClient::socketEstablished(QuicSocket *socket) {
    EV_DEBUG << "Connection established" << endl;

    Packet *packet = new Packet("ApplicationData");
    int sendBytes = 1000;
    auto applicationData = makeShared<ByteCountChunk>(B(sendBytes), '?');
    //applicationData->addTag<CreationTimeTag>()->setCreationTime(simTime());
    packet->insertAtBack(applicationData);
    // set streamId for applicationData
    auto& tags = packet->getTags();
    tags.addTagIfAbsent<QuicStreamReq>()->setStreamID(0);
    socket->send(packet);

    packet = new Packet("ApplicationData");
    sendBytes = 500;
    applicationData = makeShared<ByteCountChunk>(B(sendBytes), '!');
    //applicationData->addTag<CreationTimeTag>()->setCreationTime(simTime());
    packet->insertAtBack(applicationData);
    // set streamId for applicationData
    auto& tags2 = packet->getTags();
    tags2.addTagIfAbsent<QuicStreamReq>()->setStreamID(0);
    socket->send(packet);

    if (socket == &socket2) {
        //socket->close();
    }
}

void QuicZeroRttClient::socketDataArrived(QuicSocket* socket, Packet *packet) {
    EV_DEBUG << "Data arrived" << endl;
    delete packet;
}

void QuicZeroRttClient::socketClosed(QuicSocket *socket) {
    EV_DEBUG << "Socket closed" << endl;
}

void QuicZeroRttClient::socketNewToken(QuicSocket *socket, const char *token)
{
    EV_DEBUG << "Got token " << token << endl;

    if (socket == &socket1) {
        EV_DEBUG << "Close socket1 and use socket2 for 0-RTT connection setup" << endl;
        socket1.close();

        L3Address localAddress = L3AddressResolver().resolve(par("localAddress"));
        int localPort = par("localPort");
        L3Address connectAddress = L3AddressResolver().resolve(par("connectAddress"));
        int connectPort = par("connectPort");

        socket2.setOutputGate(gate("socketOut"));
        socket2.bind(localAddress, localPort);
        socket2.setCallback(this);

        Packet *packet = new Packet("ApplicationData");
        int sendBytes = 1500;
        auto applicationData = makeShared<ByteCountChunk>(B(sendBytes), '#');
        packet->insertAtBack(applicationData);
        uint64_t streamId = 0;

        if (par("invalidClientTokenString")) {
            socket2.connectAndSend(connectAddress, connectPort, "", packet, streamId);
        } else if (par("sendInvalidToken")) {
            char invalidToken[255]; // 255 characters hopefully long enough
            strcpy(invalidToken, token);
            if (invalidToken[0] != '0') {
                invalidToken[0] = '0';
            } else {
                invalidToken[0] = '1';
            }
            socket2.connectAndSend(connectAddress, connectPort, invalidToken, packet, streamId);
        } else {
            socket2.connectAndSend(connectAddress, connectPort, token, packet, streamId);
        }
    }

}

} //namespace
