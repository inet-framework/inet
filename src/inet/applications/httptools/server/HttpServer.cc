//
// Copyright (C) 2009 Kristjan V. Jonsson, LDSS (kristjanvj@gmail.com)
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License version 3
// as published by the Free Software Foundation.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

#include "inet/applications/httptools/server/HttpServer.h"

namespace inet {

namespace httptools {

Define_Module(HttpServer);

void HttpServer::initialize(int stage)
{
    HttpServerBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        numBroken = 0;
        socketsOpened = 0;

        WATCH(numBroken);
        WATCH(socketsOpened);
    }
    else if (stage == INITSTAGE_APPLICATION_LAYER) {
        EV_DEBUG << "Initializing server component (sockets version)" << endl;

        int port = par("port");

        listensocket.setOutputGate(gate("socketOut"));
        listensocket.bind(port);
        listensocket.setCallback(this);
        listensocket.listen();
    }
}

void HttpServer::finish()
{
    HttpServerBase::finish();

    EV_INFO << "Sockets opened: " << socketsOpened << endl;
    EV_INFO << "Broken connections: " << numBroken << endl;

    recordScalar("sock.opened", socketsOpened);
    recordScalar("sock.broken", numBroken);

    // Clean up sockets and data structures
    sockCollection.deleteSockets();
}

void HttpServer::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage()) {
        // Self messages not used at the moment
    }
    else {
        EV_DEBUG << "Handle inbound message " << msg->getName() << " of kind " << msg->getKind() << endl;
        TcpSocket *socket = check_and_cast_nullable<TcpSocket*>(sockCollection.findSocketFor(msg));
        if (socket) {
            EV_DEBUG << "Process the message " << msg->getName() << endl;
            socket->processMessage(msg);
        }
        else {
            EV_DEBUG << "No socket found for the message. Create a new one" << endl;
            // new connection -- create new socket object and server process
            socket = new TcpSocket(msg);
            socket->setOutputGate(gate("socketOut"));
            sockCollection.addSocket(socket);

            // Initialize the associated data structure
            SockData *sockdata = new SockData;
            sockdata->socket = socket;

            socket->setCallback(this);
            socket->setUserData(sockdata);
            listensocket.processMessage(msg);
        }
    }
}

void HttpServer::socketEstablished(TcpSocket *socket)
{
    EV_INFO << "connected socket with id=" << socket->getSocketId() << endl;
    socketsOpened++;
}

void HttpServer::socketDataArrived(TcpSocket *socket)
{
    SockData *sockdata = (SockData *)socket->getUserData();
    if (sockdata == nullptr) {
        EV_ERROR << "Socket establish failure. Null pointer" << endl;
        return;
    }

    // Should be a HttpReplyMessage
    EV_DEBUG << "Socket data arrived on connection " << socket->getSocketId() << "." << endl;

    // call the message handler to process the message.
    auto queue = socket->getReceiveQueue();
    while (queue->has<HttpRequestMessage>()) {
        auto packet = new Packet("", queue->pop<HttpRequestMessage>());
        auto reply = handleReceivedMessage(packet);
        if (reply != nullptr)
            socket->send(reply);    // Send to socket if the reply is non-zero.
        delete packet;
    }
}

void HttpServer::socketPeerClosed(TcpSocket *socket)
{
    SockData *sockdata = (SockData *)socket->getUserData();
    if (sockdata == nullptr) {
        EV_ERROR << "Socket establish failure. Null pointer" << endl;
        return;
    }
    ASSERT(socket == sockdata->socket);

    // close the connection (if not already closed)
    if (socket->getState() == TcpSocket::PEER_CLOSED) {
        EV_INFO << "remote TCP closed, closing here as well. Connection id is " << socket->getSocketId() << endl;
        socket->close();    // Call the close method to properly dispose of the socket.
    }
}

void HttpServer::socketClosed(TcpSocket *socket)
{
    EV_INFO << "connection closed. Connection id " << socket->getSocketId() << endl;

    SockData *sockdata = (SockData *)socket->getUserData();
    if (sockdata == nullptr) {
        EV_ERROR << "Socket establish failure. Null pointer" << endl;
    }
    else {
        ASSERT(socket == sockdata->socket);
        // Cleanup
        sockCollection.removeSocket(socket);
    }
    delete socket;
}

void HttpServer::socketFailure(TcpSocket *socket, int code)
{
    int connId = socket->getSocketId();
    EV_WARN << "connection broken. Connection id " << connId << endl;
    numBroken++;

    EV_INFO << "connection closed. Connection id " << connId << endl;

    SockData *sockdata = (SockData *)socket->getUserData();
    if (sockdata == nullptr) {
        EV_ERROR << "Socket establish failure. Null pointer" << endl;
        return;
    }
    ASSERT(socket == sockdata->socket);

    if (code == TCP_I_CONNECTION_RESET)
        EV_WARN << "Connection reset!\n";
    else if (code == TCP_I_CONNECTION_REFUSED)
        EV_WARN << "Connection refused!\n";

    // Cleanup
    sockCollection.removeSocket(socket);
    delete socket;
}

void HttpServer::socketDeleted(TcpSocket *socket)
{
    SockData *sockdata = (SockData *)socket->getUserData();
    delete sockdata;
}

} // namespace httptools

} // namespace inet

