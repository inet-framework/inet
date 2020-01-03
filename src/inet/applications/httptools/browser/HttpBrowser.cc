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

#include "inet/common/packet/Message.h"
#include "inet/applications/common/SocketTag_m.h"
#include "inet/applications/httptools/browser/HttpBrowser.h"

namespace inet {

namespace httptools {

Define_Module(HttpBrowser);

HttpBrowser::HttpBrowser()
{
}

HttpBrowser::~HttpBrowser()
{
    // @todo Delete socket data structures
    sockCollection.deleteSockets();
}

void HttpBrowser::initialize(int stage)
{
    EV_DEBUG << "Initializing HTTP browser component (sockets version), stage " << stage << endl;
    HttpBrowserBase::initialize(stage);
}

void HttpBrowser::finish()
{
    // Call the parent class finish. Takes care of most of the reporting.
    HttpBrowserBase::finish();

    // Report sockets related statistics.
    EV_INFO << "Sockets opened: " << socketsOpened << endl;
    EV_INFO << "Broken connections: " << numBroken << endl;
    // Record the sockets related statistics
    recordScalar("sock.opened", socketsOpened);
    recordScalar("sock.broken", numBroken);
}

void HttpBrowser::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage()) {
        handleSelfMessages(msg);
    }
    else {
        EV_DEBUG << "Message received: " << msg->getName() << endl;

        TcpCommand *ind = dynamic_cast<TcpCommand *>(msg->getControlInfo());
        if (!ind) {
            EV_DEBUG << "No control info for the message" << endl;
        }
        else {
            auto indication = check_and_cast<Indication *>(msg);
            int connId = indication->getTag<SocketInd>()->getSocketId();
            EV_DEBUG << "Connection ID: " << connId << endl;
        }

        // Locate the socket for the incoming message. One should definitely exist.
        TcpSocket *socket = check_and_cast_nullable<TcpSocket*>(sockCollection.findSocketFor(msg));
        if (socket == nullptr) {
            // Handle errors. @todo error instead of warning?
            EV_WARN << "No socket found for message " << msg->getName() << endl;
            delete msg;
            return;
        }
        // Submit to the socket handler. Calls the TcpSocket::ICallback methods.
        // Message is deleted in the socket handler
        socket->processMessage(msg);
    }
}

void HttpBrowser::sendRequestToServer(BrowseEvent be)
{
    int connectPort;
    char szModuleName[127];

    if (controller->getServerInfo(be.wwwhost.c_str(), szModuleName, connectPort) != 0) {
        EV_ERROR << "Unable to get server info for URL " << be.wwwhost << endl;
        return;
    }

    EV_DEBUG << "Sending request to server " << be.wwwhost << " (" << szModuleName << ") on port " << connectPort << endl;
    submitToSocket(szModuleName, connectPort, generatePageRequest(be.wwwhost, be.resourceName));
}

void HttpBrowser::sendRequestToServer(Packet *pk)
{
    const auto& request = pk->peekAtFront<HttpRequestMessage>();
    int connectPort;
    char szModuleName[127];

    if (controller->getServerInfo(request->getTargetUrl(), szModuleName, connectPort) != 0) {
        EV_ERROR << "Unable to get server info for URL " << request->getTargetUrl() << endl;
        delete pk;
        return;
    }

    EV_DEBUG << "Sending request to server " << request->getTargetUrl() << " (" << szModuleName << ") on port " << connectPort << endl;
    submitToSocket(szModuleName, connectPort, pk);
}

void HttpBrowser::sendRequestToRandomServer()
{
    int connectPort;
    char szWWW[127];
    char szModuleName[127];

    if (controller->getAnyServerInfo(szWWW, szModuleName, connectPort) != 0) {
        EV_ERROR << "Unable to get a random server from controller" << endl;
        return;
    }

    EV_DEBUG << "Sending request to random server " << szWWW << " (" << szModuleName << ") on port " << connectPort << endl;
    submitToSocket(szModuleName, connectPort, generateRandomPageRequest(szWWW));
}

void HttpBrowser::sendRequestsToServer(std::string www, HttpRequestQueue queue)
{
    int connectPort;
    char szModuleName[127];

    if (controller->getServerInfo(www.c_str(), szModuleName, connectPort) != 0) {
        EV_ERROR << "Unable to get server info for URL " << www << endl;
        while (!queue.empty()) {
            Packet *msg = queue.back();
            queue.pop_back();
            delete msg;
        }
        return;
    }

    EV_DEBUG << "Sending requests to server " << www << " (" << szModuleName << ") on port " << connectPort
             << ". Total messages queued are " << queue.size() << endl;
    submitToSocket(szModuleName, connectPort, queue);
}

void HttpBrowser::socketEstablished(TcpSocket *socket)
{
    int connId = socket->getSocketId();
    EV_DEBUG << "Socket with id " << connId << " established" << endl;

    socketsOpened++;

    // Get the socket and associated data structure.
    SockData *sockdata = (SockData *)socket->getUserData();
    if (sockdata == nullptr) {
        EV_ERROR << "SocketEstablished failure. Null pointer" << endl;
        return;
    }
    ASSERT(socket == sockdata->socket);
    if (sockdata->messageQueue.empty()) {
        EV_INFO << "No data to send on socket with connection id " << connId << ". Closing" << endl;
        socket->close();
        return;
    }

    // Send pending messages on the established socket.
    EV_DEBUG << "Proceeding to send messages on socket " << connId << endl;
    while (!sockdata->messageQueue.empty()) {
        cMessage *msg = sockdata->messageQueue.back();
        Packet *pckt = check_and_cast<Packet *>(msg);
        sockdata->messageQueue.pop_back();
        EV_DEBUG << "Submitting request " << msg->getName() << " to socket " << connId << ". size is " << pckt->getByteLength() << " bytes" << endl;
        socket->send(pckt);
        sockdata->pending++;
    }
}

void HttpBrowser::socketDataArrived(TcpSocket *socket)
{
    EV_DEBUG << "Socket data arrived on connection " << socket->getSocketId() << endl;
    SockData *sockdata = (SockData *)socket->getUserData();
    if (sockdata == nullptr) {
        EV_ERROR << "socketDataArrivedfailure. Null pointer" << endl;
        return;
    }

    ASSERT(socket == sockdata->socket);
    auto queue = socket->getReceiveQueue();
    while (queue->has<HttpReplyMessage>())
        handleDataMessage(queue->pop<HttpReplyMessage>());

    if (--sockdata->pending == 0) {
        EV_DEBUG << "Received last expected reply on this socket. Issuing a close" << endl;
        socket->close();
    }
    // Message deleted in handler - do not delete here!
}

void HttpBrowser::socketStatusArrived(TcpSocket *socket, TcpStatusInfo *status)
{
    // This is obviously not used at the present time.
    EV_INFO << "SOCKET STATUS ARRIVED. Socket: " << socket->getSocketId() << endl;
}

void HttpBrowser::socketPeerClosed(TcpSocket *socket)
{
    int connId = socket->getSocketId();
    EV_DEBUG << "Socket " << connId << " closed by peer" << endl;
    SockData *sockdata = (SockData *)socket->getUserData();
    if (sockdata == nullptr) {
        EV_ERROR << "socketPeerClosed failure. Null pointer" << endl;
        return;
    }

    ASSERT(socket == sockdata->socket);

    // close the connection (if not already closed)
    if (socket->getState() == TcpSocket::PEER_CLOSED) {
        EV_INFO << "remote TCP closed, closing here as well. Connection id is " << connId << endl;
        socket->close();
    }
}

void HttpBrowser::socketClosed(TcpSocket *socket)
{
    int connId = socket->getSocketId();
    EV_INFO << "Socket " << connId << " closed" << endl;

    SockData *sockdata = (SockData *)socket->getUserData();
    if (sockdata == nullptr) {
        EV_ERROR << "socketClosed failure. Null pointer" << endl;
    }
    else {
        ASSERT(socket == sockdata->socket);
        sockCollection.removeSocket(socket);
    }
    delete socket;
}

void HttpBrowser::socketFailure(TcpSocket *socket, int code)
{
    int connId = socket->getSocketId();
    EV_WARN << "connection broken. Connection id " << connId << endl;
    numBroken++;

    SockData *sockdata = (SockData *)socket->getUserData();
    if (sockdata == nullptr) {
        EV_ERROR << "socketFailure failure. Null pointer" << endl;
        return;
    }

    if (code == TCP_I_CONNECTION_RESET)
        EV_WARN << "Connection reset!\n";
    else if (code == TCP_I_CONNECTION_REFUSED)
        EV_WARN << "Connection refused!\n";

    ASSERT(socket == sockdata->socket);
    sockCollection.removeSocket(socket);
    delete socket;
}

void HttpBrowser::socketDeleted(TcpSocket *socket)
{
    SockData *sockdata = (SockData *)socket->getUserData();
    if (sockdata == nullptr) {
        throw cRuntimeError("Model error: socketDelete failure. yourPtr is null pointer");
    }

    // TODO: socket is already deleted, no?
    ASSERT(socket == sockdata->socket);
    HttpRequestQueue& queue = sockdata->messageQueue;
    while (!queue.empty()) {
        Packet *msg = queue.back();
        queue.pop_back();
        delete msg;
    }
    delete sockdata;
}

void HttpBrowser::submitToSocket(const char *moduleName, int connectPort, Packet *msg)
{
    // Create a queue and push the single message
    HttpRequestQueue queue;
    queue.push_back(msg);
    // Call the overloaded version with the queue as parameter
    submitToSocket(moduleName, connectPort, queue);
}

void HttpBrowser::submitToSocket(const char *moduleName, int connectPort, HttpRequestQueue& queue)
{
    // Don't do anything if the queue is empty.
    if (queue.empty()) {
        EV_INFO << "Submitting to socket. No data to send to " << moduleName << ". Skipping connection." << endl;
        return;
    }

    EV_DEBUG << "Submitting to socket. Module: " << moduleName << ", port: " << connectPort << ". Total messages: " << queue.size() << endl;

    // Create and initialize the socket
    TcpSocket *socket = new TcpSocket();
    socket->setOutputGate(gate("socketOut"));
    sockCollection.addSocket(socket);

    // Initialize the associated data structure
    SockData *sockdata = new SockData;
    sockdata->messageQueue = HttpRequestQueue(queue);
    sockdata->socket = socket;
    sockdata->pending = 0;
    socket->setCallback(this);
    socket->setUserData(sockdata);

    // Issue a connect to the socket for the specified module and port.
    socket->connect(L3AddressResolver().resolve(moduleName), connectPort);
}

} // namespace httptools

} // namespace inet

