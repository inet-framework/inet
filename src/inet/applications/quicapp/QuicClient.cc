//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#include "QuicClient.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/common/packet/chunk/ByteCountChunk.h"

namespace inet {

Define_Module(QuicClient);

void QuicClient::handleStartOperation(LifecycleOperation *operation)
{
    //EV_DEBUG << "initialize QUIC Client stage " << stage << endl;

    L3Address localAddress = L3AddressResolver().resolve(par("localAddress"));
    int localPort = par("localPort");
    L3Address connectAddress = L3AddressResolver().resolve(par("connectAddress"));
    int connectPort = par("connectPort");

    socket.setOutputGate(gate("socketOut"));
    socket.bind(localAddress, localPort);
    socket.connect(connectAddress, connectPort);
    socket.setCallback(this);
    EV_INFO << "QuicClient::initialized with localPort=" << localPort << ", connectPort=" << connectPort << endl;

}

void QuicClient::handleStopOperation(LifecycleOperation *operation)
{
    socket.close();
}

void QuicClient::handleCrashOperation(LifecycleOperation *operation)
{
}

void QuicClient::handleMessageWhenUp(cMessage *msg)
{
    EV_DEBUG << "handle message of kind " << msg->getKind() << endl;
    if (msg->isSelfMessage()) {
        EV_DEBUG << "SelfMessage received" << endl;
    } else {
        socket.processMessage(msg);
    }
}

void QuicClient::socketEstablished(QuicSocket *socket) {
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

    socket->close();
}

void QuicClient::socketDataArrived(QuicSocket* socket, Packet *packet) {
    EV_DEBUG << "Data arrived" << endl;
}

void QuicClient::socketClosed(QuicSocket *socket) {
    EV_DEBUG << "Socket closed" << endl;
}

} //namespace
