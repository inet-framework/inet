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

#ifndef __INET_HTTPBROWSER_H
#define __INET_HTTPBROWSER_H

#include "inet/transportlayer/contract/tcp/TCPSocket.h"
#include "inet/transportlayer/contract/tcp/TCPSocketMap.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/applications/httptools/browser/HttpBrowserBase.h"

namespace inet {

namespace httptools {

/**
 * Browser module.
 *
 * The component is designed to plug into the existing INET StandardHost module as a
 * tcpApp. See the INET documentation and examples for details.
 *
 * This component uses the TCP/IP modeling of the INET framework for transport.
 * Specifically, the TCPSocket class is used to interface with the TCP component from the INET framework.
 * A light-weight module which uses direct message passing is also available (HttpBrowserDirect).
 *
 * @author Kristjan V. Jonsson (kristjanvj@gmail.com)
 *
 * @see HttpBrowserBase
 * @see HttpBrowserDirect
 */
class INET_API HttpBrowser : public HttpBrowserBase, public TCPSocket::CallbackInterface
{
  protected:
    /*
     * Data structure used to keep state for each opened socket.
     *
     * An instance of this struct is created for each opened socket and assigned to
     * it as a myPtr. See the TCPSocket::CallbackInterface methods of HttpBrowser for more
     * details.
     */
    struct SockData
    {
        HttpRequestQueue messageQueue;    // Queue of pending messages.
        TCPSocket *socket;    // A reference to the socket object.
        int pending;    // A counter for the number of outstanding replies.
    };

    TCPSocketMap sockCollection;    // List of active sockets
    unsigned long numBroken;    // Counter for the number of broken connections
    unsigned long socketsOpened;    // Counter for opened sockets

  protected:
    virtual void initialize(int stage);
    virtual void finish();
    virtual void handleMessage(cMessage *msg);
    int numInitStages() const { return NUM_INIT_STAGES; }

    /*
     * Sends a scripted browse event to a specific server
     */
    virtual void sendRequestToServer(BrowseEvent be);

    /*
     * Send a request to server. Uses the recipient stamped in the request.
     */
    virtual void sendRequestToServer(HttpRequestMessage *request);

    /*
     * Sends a generic request to a randomly chosen server
     */
    virtual void sendRequestToRandomServer();

    /*
     *  Sends a number of queued messages to the specified server
     */
    virtual void sendRequestsToServer(std::string www, HttpRequestQueue queue);

    // TCPSocket::CallbackInterface callback methods
    /*
     * Handler for socket established event.
     * Called by the socket->processMessage(msg) handler call in handleMessage.
     * The pending messages for the socket are transmitted in the order queued. The socket remains
     * open after the handler has completed. A counter for pending messages is incremented for each
     * request sent.
     */
    virtual void socketEstablished(int connId, void *yourPtr);

    /*
     * Handler for socket data arrival.
     * Called by the socket->processMessage(msg) handler call in handleMessage.
     * virtual method of the parent class. The counter for pending replies is decremented for each one handled.
     * Close is called on the socket once the counter reaches zero.
     */
    virtual void socketDataArrived(int connId, void *yourPtr, cPacket *msg, bool urgent);

    /*
     * Handler for the socket closed by peer event.
     * Called by the socket->processMessage(msg) handler call in handleMessage.
     */
    virtual void socketPeerClosed(int connId, void *yourPtr);

    /*
     * Socket closed handler.
     * Called by the socket->processMessage(msg) handler call in handleMessage.
     */
    virtual void socketClosed(int connId, void *yourPtr);

    /*
     * Socket failure handler.
     * This method does nothing but reporting and statistics collection at this time.
     * @todo Implement reconnect if necessary. See the INET demos.
     */
    virtual void socketFailure(int connId, void *yourPtr, int code);

    /*
     * Socket status arrived handler.
     * Called by the socket->processMessage(msg) handler call in handleMessage.
     */
    virtual void socketStatusArrived(int connId, void *yourPtr, TCPStatusInfo *status);

    virtual void socketDeleted(int connId, void *yourPtr);

    // Socket establishment and data submission
    /*
     * Establishes a socket and queues a single message for transmission.
     * A new socket is created and added to the collection. The message is assigned to a data structure
     * stored as a myPtr with the socket. The message is transmitted once the socket is established, signaled
     * by a call to socketEstablished.
     */
    void submitToSocket(const char *moduleName, int connectPort, HttpRequestMessage *msg);

    /**
     * Establishes a socket and assigns a queue of messages to be transmitted.
     * Same as the overloaded version, except a number of messages are queued for transmission. The same socket
     * instance is used for all the queued messages.
     */
    void submitToSocket(const char *moduleName, int connectPort, HttpRequestQueue& queue);

  public:
    HttpBrowser();
    virtual ~HttpBrowser();
};

} // namespace httptools

} // namespace inet

#endif // ifndef __INET_HTTPBROWSER_H

