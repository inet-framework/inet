//
// Copyright (C) 2018 OpenSim Ltd.
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

#ifndef __INET_LLCSOCKET_H
#define __INET_LLCSOCKET_H

#include "inet/common/INETDefs.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/socket/ISocket.h"

namespace inet {

class INET_API Ieee8022LlcSocket : public ISocket
{
  public:
    class INET_API ICallback
    {
      public:
        virtual ~ICallback() {}
        virtual void socketDataArrived(Ieee8022LlcSocket *socket, Packet *packet) = 0;
        virtual void socketClosed(Ieee8022LlcSocket *socket) = 0;
    };
  protected:
    int socketId = -1;
    int interfaceId = -1;
    int localSap = -1;
    ICallback *callback = nullptr;
    void *userData = nullptr;
    cGate *outputGate = nullptr;
    bool isOpen_ = false;

  protected:
    void sendToLlc(cMessage *msg);

  public:
    Ieee8022LlcSocket();
    ~Ieee8022LlcSocket() {}

    /**
     * Sets the gate on which to send raw packets. Must be invoked before socket
     * can be used. Example: <tt>socket.setOutputGate(gate("ipOut"));</tt>
     */
    void setOutputGate(cGate *outputGate) { this->outputGate = outputGate; }

    /**
     * Sets a callback object, to be used with processMessage().
     * This callback object may be your simple module itself (if it
     * multiply inherits from ICallback too, that is you
     * declared it as
     * <pre>
     * class MyAppModule : public cSimpleModule, public LlcSocket::ICallback
     * </pre>
     * and redefined the necessary virtual functions; or you may use
     * dedicated class (and objects) for this purpose.
     *
     * LlcSocket doesn't delete the callback object in the destructor
     * or on any other occasion.
     */
    void setCallback(ICallback *cb);

    void *getUserData() const { return userData; }
    void setUserData(void *userData) { this->userData = userData; }

    /**
     * Returns the internal socket Id.
     */
    virtual int getSocketId() const override { return socketId; }

    /**
     * Returns the interface Id.
     */
    virtual int getInterfaceId() const { return interfaceId; }

    /**
     * Returns the local Sap Id.
     */
    virtual int getLocalSap() const { return localSap; }

    void open(int interfaceId, int localSap);
    void send(Packet *packet);
    virtual void close() override;
    virtual bool isOpen() const override { return isOpen_; }

    virtual bool belongsToSocket(cMessage *msg) const override;
    virtual void processMessage(cMessage *msg) override;
    virtual void destroy() override;
};

} // namespace inet

#endif // ifndef __INET_LLCSOCKET_H

