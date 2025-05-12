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

#include "QuicDiscardServer.h"
#include "inet/networklayer/common/L3AddressResolver.h"

namespace inet {

Define_Module(QuicDiscardServer);

QuicDiscardServer::~QuicDiscardServer()
{
    for (QuicSocket *clientSocket : clientSockets) {
        delete clientSocket;
    }
}

void QuicDiscardServer::handleStartOperation(LifecycleOperation *operation)
{
    L3Address localAddress = L3AddressResolver().resolve(par("localAddress"));
    int localPort = par("localPort");

    listeningSocket.setOutputGate(gate("socketOut"));
    listeningSocket.bind(localAddress, localPort);
    listeningSocket.listen();
    EV_INFO << "listen over socket " << listeningSocket.getSocketId() << " on port " << localPort << endl;
    listeningSocket.setCallback(this);
}

void QuicDiscardServer::handleStopOperation(LifecycleOperation *operation)
{
    listeningSocket.close();
    for (QuicSocket *clientSocket : clientSockets) {
        clientSocket->close();
    }
}

void QuicDiscardServer::handleCrashOperation(LifecycleOperation *operation)
{

}

void QuicDiscardServer::handleMessageWhenUp(cMessage *msg)
{
    EV_DEBUG << "handle message of kind " << msg->getKind() << endl;
    if (msg->isSelfMessage()) {
        EV_DEBUG << "SelfMessage received" << endl;
    } else {
        if (listeningSocket.belongsToSocket(msg)) {
            listeningSocket.processMessage(msg);
        } else {
            for (QuicSocket *clientSocket : clientSockets) {
                if (clientSocket->belongsToSocket(msg)) {
                    clientSocket->processMessage(msg);
                    break;
                }
            }
        }
    }
}

void QuicDiscardServer::socketDataArrived(QuicSocket* socket, Packet *packet)
{
    auto data = packet->popAtFront();
    static simsignal_t bytesReceivedSignal = registerSignal("bytesReceived");
    emit(bytesReceivedSignal, (long)B(data->getChunkLength()).get());
    EV_DEBUG << data << " received and discarded over socket " << socket->getSocketId() << endl;
    delete packet;
}

void QuicDiscardServer::socketConnectionAvailable(QuicSocket *socket)
{
    EV_DEBUG << "connection available on socket " << socket->getSocketId() << " - call accept" << endl;
    QuicSocket *clientSocket = socket->accept();
    clientSocket->setCallback(this);
    EV_DEBUG << "got client socket " << clientSocket->getSocketId() << endl;
    clientSockets.push_back(clientSocket);
}

void QuicDiscardServer::socketDataAvailable(QuicSocket* socket, QuicDataInfo *dataInfo)
{
    auto streamId = dataInfo->getStreamID();
    auto avaliableDataSize = dataInfo->getAvaliableDataSize();
    EV_DEBUG << avaliableDataSize << " bytes arrived over socket " << socket->getSocketId() << " on stream " << streamId << " - call recv" << endl;
    socket->recv(avaliableDataSize, streamId);
}

void QuicDiscardServer::socketEstablished(QuicSocket *socket)
{
    EV_DEBUG << "connection to " << socket->getRemoteAddr() << ":" << socket->getRemotePort() << " established" << endl;
}

void QuicDiscardServer::socketClosed(QuicSocket *socket)
{
    EV_DEBUG << "connection closed over socket " << socket->getSocketId() << endl;
}

void QuicDiscardServer::socketDestroyed(QuicSocket *socket)
{
    EV_DEBUG << "socket " << socket->getSocketId() << " destroyed" << endl;
    if (socket != &listeningSocket) {
        for (std::vector<QuicSocket *>::iterator it = clientSockets.begin(); it != clientSockets.end(); ++it) {
            if (*it == socket) {
                clientSockets.erase(it);
                break;
            }
        }
        delete socket;
    }
}

} //namespace
