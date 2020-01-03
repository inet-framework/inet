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

#ifndef __INET_EXTLOWERUDP_H
#define __INET_EXTLOWERUDP_H

#include "inet/common/lifecycle/ILifecycle.h"
#include "inet/common/packet/printer/PacketPrinter.h"
#include "inet/common/scheduler/RealTimeScheduler.h"
#include "inet/networklayer/common/L3Address.h"

namespace inet {

class INET_API ExtLowerUdp : public cSimpleModule, public ILifecycle, public RealTimeScheduler::ICallback
{
  protected:
    class Socket
    {
      public:
        int socketId = -1;
        int fd = -1;

      public:
        Socket(int socketId) : socketId(socketId) {}
    };

    const char *packetNameFormat = nullptr;
    PacketPrinter packetPrinter;
    RealTimeScheduler *rtScheduler = nullptr;
    std::map<int, Socket *> socketIdToSocketMap;
    std::map<int, Socket *> fdToSocketMap;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *message) override;
    virtual bool handleOperationStage(LifecycleOperation *operation, IDoneCallback *doneCallback) override;
    virtual bool notify(int fd) override;

    virtual Socket *open(int socketId);
    virtual void bind(int socketId, const L3Address& localAddress, int localPort);
    virtual void connect(int socketId, const L3Address& remoteAddress, int remotePort);
    virtual void close(int socketId);
    virtual void processPacketFromUpper(Packet *packet);
    virtual void processPacketFromLower(int fd);

  public:
    virtual ~ExtLowerUdp();
};

} // namespace inet

#endif // ifndef __INET_EXTLOWERUDP_H

