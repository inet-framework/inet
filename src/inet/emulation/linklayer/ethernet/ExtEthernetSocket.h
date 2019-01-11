//
// Copyright (C) OpenSim Ltd.
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

#ifndef __INET_EXTETHERNETSOCKET_H
#define __INET_EXTETHERNETSOCKET_H

#include "inet/common/packet/printer/PacketPrinter.h"
#include "inet/common/scheduler/RealTimeScheduler.h"

namespace inet {

class INET_API ExtEthernetSocket : public cSimpleModule, public RealTimeScheduler::ICallback
{
  protected:
    // parameters
    const char *device = nullptr;
    const char *packetNameFormat = nullptr;
    RealTimeScheduler *rtScheduler = nullptr;

    // statistics
    int numSent = 0;
    int numReceived = 0;

    // state
    PacketPrinter packetPrinter;
    int ifindex = -1;
    int fd = INVALID_SOCKET;
    MacAddress macAddress;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *message) override;
    virtual void refreshDisplay() const override;
    virtual void finish() override;

    virtual void openSocket();
    virtual void closeSocket();

  public:
    virtual ~ExtEthernetSocket();

    virtual bool notify(int fd) override;
};

} // namespace inet

#endif // ifndef __INET_EXTETHERNETSOCKET_H

