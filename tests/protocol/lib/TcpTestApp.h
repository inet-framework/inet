//
// Protocol Test Framework for INET -- an application whose TCP sockets a test program drives.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_PROTOCOLTEST_TCPTESTAPP_H
#define __INET_PROTOCOLTEST_TCPTESTAPP_H

#include <vector>

#include "inet/common/SimpleModule.h"
#include "inet/common/socket/SocketMap.h"
#include "inet/transportlayer/contract/tcp/TcpSocket.h"

namespace inet {
namespace protocoltest {

//
// The sockets of a test: one socket for an active open or for a listener, and the sockets
// that the listener forked. The program calls them in an action; see TcpTestApp.ned.
//
//     .act(action().at(0.0).run<TcpTestApp>("host2.app[0]", [](TcpTestApp& app) { app.socket().bind(1000); app.socket().listen(); }))
//     .act(action().at(0.1).run<TcpTestApp>("host1.app[0]", [](TcpTestApp& app) { app.socket().connect(app.addressOf("host2"), 1000, true); }))
//
class INET_API TcpTestApp : public SimpleModule, public TcpSocket::ICallback
{
  protected:
    TcpSocket mainSocket;
    std::vector<TcpSocket *> acceptedSockets;
    SocketMap socketMap;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override;

    virtual void socketDataArrived(TcpSocket *socket, Packet *packet, bool urgent) override;
    virtual void socketAvailable(TcpSocket *socket, TcpAvailableInfo *availableInfo) override;
    virtual void socketEstablished(TcpSocket *socket) override;
    virtual void socketPeerClosed(TcpSocket *socket) override;
    virtual void socketClosed(TcpSocket *socket) override;
    virtual void socketFailure(TcpSocket *socket, int code) override;
    virtual void socketStatusArrived(TcpSocket *socket, TcpStatusInfo *status) override { delete status; }
    virtual void socketDeleted(TcpSocket *socket) override {}
    virtual void socketSendMsgArrived(TcpSocket *socket, TcpCommand *tcpCommand) override;

  public:
    virtual ~TcpTestApp();

    // The socket of an active open, or the listening socket.
    TcpSocket& socket() { return mainSocket; }
    // A socket that the listener forked, in the order of their arrival.
    TcpSocket& accepted(int index = 0) { return *acceptedSockets.at(index); }
    int getNumAccepted() const { return acceptedSockets.size(); }

    // The address of a node, by its name.
    L3Address addressOf(const char *nodeName) const;
    // Sends this many bytes of data on a socket, with the EOR flag if asked.
    void sendData(TcpSocket& socket, int64_t bytes, bool eor = false);
};

} // namespace protocoltest
} // namespace inet

#endif
