// ***************************************************************************
//
// HttpTools Project
//
// This file is a part of the HttpTools project. The project was created at
// Reykjavik University, the Laboratory for Dependable Secure Systems (LDSS).
// Its purpose is to create a set of OMNeT++ components to simulate browsing
// behaviour in a high-fidelity manner along with a highly configurable
// Web server component.
//
// Maintainer: Kristjan V. Jonsson (LDSS) kristjanvj@gmail.com
// Project home page: code.google.com/p/omnet-httptools
//
// ***************************************************************************
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
// ***************************************************************************

#include "HttpServer.h"

Define_Module(HttpServer);

void HttpServer::initialize()
{
    HttpServerBase::initialize();

    EV_DEBUG << "Initializing server component (sockets version)" << endl;

    int port = par("port");

    TCPSocket listensocket;
    listensocket.setOutputGate(gate("tcpOut"));
    listensocket.setDataTransferMode(TCP_TRANSFER_OBJECT);
    listensocket.bind(port);
    listensocket.setCallbackObject(this);
    listensocket.listen();

    numBroken = 0;
    socketsOpened = 0;

    WATCH(numBroken);
    WATCH(socketsOpened);
}

void HttpServer::finish()
{
    HttpServerBase::finish();

    EV_SUMMARY << "Sockets opened: " << socketsOpened << endl;
    EV_SUMMARY << "Broken connections: " << numBroken << endl;

    recordScalar("sock.opened", socketsOpened);
    recordScalar("sock.broken", numBroken);

    // Clean up sockets and data structures
    sockCollection.deleteSockets();
}

void HttpServer::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage())
    {
        // Self messages not used at the moment
    }
    else
    {
        EV_DEBUG << "Handle inbound message " << msg->getName() << " of kind " << msg->getKind() << endl;
        TCPSocket *socket = sockCollection.findSocketFor(msg);
        if (!socket)
        {
            EV_DEBUG << "No socket found for the message. Create a new one" << endl;
            // new connection -- create new socket object and server process
            socket = new TCPSocket(msg);
            socket->setOutputGate(gate("tcpOut"));
            socket->setDataTransferMode(TCP_TRANSFER_OBJECT);
            socket->setCallbackObject(this, socket);
            sockCollection.addSocket(socket);
        }
        EV_DEBUG << "Process the message " << msg->getName() << endl;
        socket->processMessage(msg);
    }
    updateDisplay();
}

void HttpServer::socketEstablished(int connId, void *yourPtr)
{
    EV_INFO << "connected socket with id=" << connId << endl;
    socketsOpened++;
}

void HttpServer::socketDataArrived(int connId, void *yourPtr, cPacket *msg, bool urgent)
{
    if (yourPtr==NULL)
    {
        EV_ERROR << "Socket establish failure. Null pointer" << endl;
        return;
    }
    TCPSocket *socket = (TCPSocket*)yourPtr;

    // Should be a HttpReplyMessage
    EV_DEBUG << "Socket data arrived on connection " << connId << ". Message=" << msg->getName() << ", kind=" << msg->getKind() << endl;

    // call the message handler to process the message.
    cMessage *reply = handleReceivedMessage(msg);
    if (reply!=NULL)
    {
        socket->send(reply); // Send to socket if the reply is non-zero.
    }
    delete msg; // Delete the received message here. Must not be deleted in the handler!
}

void HttpServer::socketPeerClosed(int connId, void *yourPtr)
{
    if (yourPtr==NULL)
    {
        EV_ERROR << "Socket establish failure. Null pointer" << endl;
        return;
    }
    TCPSocket *socket = (TCPSocket*)yourPtr;

    // close the connection (if not already closed)
    if (socket->getState()==TCPSocket::PEER_CLOSED)
    {
        EV_INFO << "remote TCP closed, closing here as well. Connection id is " << connId << endl;
        socket->close();  // Call the close method to properly dispose of the socket.
    }
}

void HttpServer::socketClosed(int connId, void *yourPtr)
{
    EV_INFO << "connection closed. Connection id " << connId << endl;

    if (yourPtr==NULL)
    {
        EV_ERROR << "Socket establish failure. Null pointer" << endl;
        return;
    }
    // Cleanup
    TCPSocket *socket = (TCPSocket*)yourPtr;
    sockCollection.removeSocket(socket);
    delete socket;
}

void HttpServer::socketFailure(int connId, void *yourPtr, int code)
{
    EV_WARNING << "connection broken. Connection id " << connId << endl;
    numBroken++;

    EV_INFO << "connection closed. Connection id " << connId << endl;

    if (yourPtr==NULL)
    {
        EV_ERROR << "Socket establish failure. Null pointer" << endl;
        return;
    }
    TCPSocket *socket = (TCPSocket*)yourPtr;

    if (code==TCP_I_CONNECTION_RESET)
        EV_WARNING << "Connection reset!\n";
    else if (code==TCP_I_CONNECTION_REFUSED)
        EV_WARNING << "Connection refused!\n";

    // Cleanup
    sockCollection.removeSocket(socket);
    delete socket;
}




