//
// Copyright (C) OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_INETWORKSOCKET_H
#define __INET_INETWORKSOCKET_H

#include "inet/common/socket/ISocket.h"

namespace inet {

class INET_API INetworkSocket : public ISocket
{
  public:
    class INET_API ICallback
    {
      public:
        virtual ~ICallback() {}
        virtual void socketDataArrived(INetworkSocket *socket, Packet *packet) = 0;
    };

  public:
    virtual void setCallback(ICallback *callback) = 0;
    virtual const Protocol *getNetworkProtocol() const = 0;

    virtual void bind(const Protocol *protocol) = 0;
    virtual void send(Packet *packet) = 0;
    virtual void close() = 0;
};

} // namespace inet

#endif // ifndef __INET_INETWORKSOCKET_H

