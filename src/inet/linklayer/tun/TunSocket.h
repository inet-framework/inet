//
// Copyright (C) 2013 OpenSim Ltd.
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

#ifndef __INET_TUNSOCKET_H
#define __INET_TUNSOCKET_H

#include "inet/common/INETDefs.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/socket/ISocket.h"

namespace inet {

class INET_API TunSocket : public ISocket
{
  public:
    class ICallback {
      public:
        virtual ~ICallback() {}
        virtual void socketDataArrived(TunSocket *socket, Packet *packet) = 0;
    };
  protected:
    int socketId = -1;
    int interfaceId = -1;
    ICallback *cb = nullptr;
    void *userData = nullptr;
    cGate *outputGate = nullptr;

  protected:
    void sendToTun(cMessage *msg);

  public:
    TunSocket();
    ~TunSocket() {}

    void setOutputGate(cGate *outputGate) { this->outputGate = outputGate; }

    void open(int interfaceId);
    void send(Packet *packet);
    void close();

    /**
     * Returns true if the message belongs to this socket instance.
     */
    virtual bool belongsToSocket(cMessage *msg) const override;

    /**
     * Sets a callback object, to be used with processMessage().
     * This callback object may be your simple module itself (if it
     * multiply inherits from ICallback too, that is you
     * declared it as
     * <pre>
     * class MyAppModule : public cSimpleModule, public TunSocket::ICallback
     * </pre>
     * and redefined the necessary virtual functions; or you may use
     * dedicated class (and objects) for this purpose.
     *
     * TunSocket doesn't delete the callback object in the destructor
     * or on any other occasion.
     */
    void setCallbackObject(ICallback *cb, void *userData = nullptr);

    virtual void processMessage(cMessage *msg) override;

    /**
     * Returns the internal socket Id.
     */
    int getSocketId() const override { return socketId; }
};

} // namespace inet

#endif // ifndef __INET_TUNSOCKET_H

