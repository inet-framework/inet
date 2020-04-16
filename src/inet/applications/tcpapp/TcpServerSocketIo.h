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

#ifndef __INET_TCPSERVERSOCKETIO_H
#define __INET_TCPSERVERSOCKETIO_H

#include "inet/transportlayer/contract/tcp/TcpSocket.h"

namespace inet {

class INET_API TcpServerSocketIo : public cSimpleModule, public TcpSocket::ICallback
{
  protected:
    TcpSocket *socket = nullptr;

  protected:
    virtual void handleMessage(cMessage *message) override;

  public:
    virtual TcpSocket *getSocket() { return socket; }
    virtual void acceptSocket(TcpAvailableInfo *availableInfo);

    virtual void socketDataArrived(TcpSocket* socket, Packet *packet, bool urgent) override;
    virtual void socketAvailable(TcpSocket *socket, TcpAvailableInfo *availableInfo) override {}
    virtual void socketEstablished(TcpSocket *socket) override {}
    virtual void socketPeerClosed(TcpSocket *socket) override {}
    virtual void socketClosed(TcpSocket *socket) override {}
    virtual void socketFailure(TcpSocket *socket, int code) override {}
    virtual void socketStatusArrived(TcpSocket *socket, TcpStatusInfo *status) override {}
    virtual void socketDeleted(TcpSocket *socket) override {}
};

} // namespace inet

#endif // ifndef __INET_TCPSERVERSOCKETIO_H

