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

#include "QuicOrderedReceiver.h"
#include "inet/common/packet/chunk/BytesChunk.h"
#include "inet/networklayer/common/L3AddressResolver.h"

namespace inet {

Define_Module(QuicOrderedReceiver);

QuicOrderedReceiver::~QuicOrderedReceiver()
{
    if (clientSocket != nullptr) {
        delete clientSocket;
    }
}

void QuicOrderedReceiver::handleStartOperation(LifecycleOperation *operation)
{
    L3Address localAddress = L3AddressResolver().resolve(par("localAddress"));
    int localPort = par("localPort");

    listeningSocket.setOutputGate(gate("socketOut"));
    listeningSocket.bind(localAddress, localPort);
    listeningSocket.listen();
    EV_INFO << "listen on port " << localPort << endl;
    listeningSocket.setCallback(this);
}

void QuicOrderedReceiver::handleStopOperation(LifecycleOperation *operation)
{
    listeningSocket.close();
    if (clientSocket != nullptr) {
        clientSocket->close();
    }
}

void QuicOrderedReceiver::handleCrashOperation(LifecycleOperation *operation)
{

}

void QuicOrderedReceiver::handleMessageWhenUp(cMessage *msg)
{
    EV_DEBUG << "handle message of kind " << msg->getKind() << endl;
    if (msg->isSelfMessage()) {
        EV_DEBUG << "SelfMessage received" << endl;
    } else {
        if (listeningSocket.belongsToSocket(msg)) {
            listeningSocket.processMessage(msg);
        } else if (clientSocket->belongsToSocket(msg)) {
            clientSocket->processMessage(msg);
        }
    }
}

void QuicOrderedReceiver::socketDataArrived(QuicSocket* socket, Packet *packet)
{
    auto data = packet->popAtFront();
    static simsignal_t bytesReceivedSignal = registerSignal("bytesReceived");
    emit(bytesReceivedSignal, (long)B(data->getChunkLength()).get());
    EV_DEBUG << data << " received, check..." << endl;
    checkData(data);
    delete packet;
}

void QuicOrderedReceiver::socketConnectionAvailable(QuicSocket *socket)
{
    assert(socket == &listeningSocket);
    EV_DEBUG << "connection available on socket - call accept" << endl;
    clientSocket = listeningSocket.accept();
    clientSocket->setCallback(this);
}

void QuicOrderedReceiver::socketDataAvailable(QuicSocket* socket, QuicDataInfo *dataInfo)
{
    assert(socket == clientSocket);
    auto streamId = dataInfo->getStreamID();
    auto avaliableDataSize = dataInfo->getAvaliableDataSize();
    EV_DEBUG << avaliableDataSize << " bytes arrived over socket " << socket->getSocketId() << " on stream " << streamId << " - call recv" << endl;
    clientSocket->recv(avaliableDataSize, streamId);
}

void QuicOrderedReceiver::socketEstablished(QuicSocket *socket)
{
    EV_DEBUG << "connection to " << socket->getRemoteAddr() << ":" << socket->getRemotePort() << " established" << endl;
}

void QuicOrderedReceiver::socketClosed(QuicSocket *socket)
{
    EV_DEBUG << "connection closed over socket " << socket->getSocketId() << endl;
}

void QuicOrderedReceiver::socketDeleted(QuicSocket *socket)
{
    EV_DEBUG << "socket " << socket->getSocketId() << " deleted" << endl;
}

void QuicOrderedReceiver::checkData(const Ptr<const Chunk> data)
{
    const Ptr<const BytesChunk> bytesChunk = dynamicPtrCast<const BytesChunk>(data);
    if (bytesChunk == nullptr) {
        EV_WARN << "Unable to check data, got " << data << endl;
        return;
    }
    for (int i=0; i<bytesChunk->getByteArraySize(); i++) {
        if (currentByte != bytesChunk->getByte(i)) {
            throw cRuntimeError("Received wrong bytes. Received byte is %d, expected byte is %d", bytesChunk->getByte(i), currentByte);
        }
        currentByte++;
    }
}

} //namespace
