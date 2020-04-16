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

#include "inet/common/INETDefs.h"

#include "inet/applications/httptools/browser/HttpBrowserBase.h"
#include "inet/common/packet/ChunkQueue.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/socket/SocketMap.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/transportlayer/contract/tcp/TcpSocket.h"

namespace inet {

namespace httptools {

/**
 * Browser module.
 *
 * The component is designed to plug into the existing INET StandardHost module as a
 * tcpApp. See the INET documentation and examples for details.
 *
 * This component uses the TCP/IP modeling of the INET framework for transport.
 * Specifically, the TcpSocket class is used to interface with the TCP component from the INET framework.
 * A light-weight module which uses direct message passing is also available (HttpBrowserDirect).
 *
 * @author Kristjan V. Jonsson (kristjanvj@gmail.com)
 *
 * @see HttpBrowserBase
 * @see HttpBrowserDirect
 */
class INET_API HttpBrowser : public HttpBrowserBase, public TcpSocket::ReceiveQueueBasedCallback
{
  protected:
    /*
     * Data structure used to keep state for each opened socket.
     *
     * An instance of this struct is created for each opened socket and assigned to
     * it as a myPtr. See the TcpSocket::ICallback methods of HttpBrowser for more
     * details.
     */
    struct SockData
    {
        HttpRequestQueue messageQueue;    // Queue of pending messages.
        TcpSocket *socket = nullptr;    // A reference to the socket object.
        int pending = 0;    // A counter for the number of outstanding replies.
    };

    SocketMap sockCollection;    // List of active sockets
    unsigned long numBroken = 0;    // Counter for the number of broken connections
    unsigned long socketsOpened = 0;    // Counter for opened sockets

  protected:
    virtual void initialize(int stage) override;
    virtual void finish() override;
    virtual void handleMessage(cMessage *msg) override;
    int numInitStages() const override { return NUM_INIT_STAGES; }

    /*
     * Sends a scripted browse event to a specific server
     */
    virtual void sendRequestToServer(BrowseEvent be) override;

    /*
     * Send a request to server. Uses the recipient stamped in the request.
     */
    virtual void sendRequestToServer(Packet *request) override;

    /*
     * Sends a generic request to a randomly chosen server
     */
    virtual void sendRequestToRandomServer() override;

    /*
     *  Sends a number of queued messages to the specified server
     */
    virtual void sendRequestsToServer(std::string www, HttpRequestQueue queue) override;

    // TcpSocket::ICallback callback methods
    virtual void socketDataArrived(TcpSocket *socket) override;
    virtual void socketAvailable(TcpSocket *socket, TcpAvailableInfo *availableInfo) override { socket->accept(availableInfo->getNewSocketId()); }
    virtual void socketEstablished(TcpSocket *socket) override;
    virtual void socketPeerClosed(TcpSocket *socket) override;
    virtual void socketClosed(TcpSocket *socket) override;
    virtual void socketFailure(TcpSocket *socket, int code) override;
    virtual void socketStatusArrived(TcpSocket *socket, TcpStatusInfo *status) override;
    virtual void socketDeleted(TcpSocket *socket) override;

    // Socket establishment and data submission
    /*
     * Establishes a socket and queues a single message for transmission.
     * A new socket is created and added to the collection. The message is assigned to a data structure
     * stored as a myPtr with the socket. The message is transmitted once the socket is established, signaled
     * by a call to socketEstablished.
     */
    void submitToSocket(const char *moduleName, int connectPort, Packet *msg);

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

