//
// Copyright (C) 2020 OpenSim Ltd.
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
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#ifndef __INET_IEEE8022LLCSOCKETIO_H
#define __INET_IEEE8022LLCSOCKETIO_H

#include "inet/applications/base/ApplicationBase.h"
#include "inet/linklayer/ieee8022/Ieee8022LlcSocket.h"
#include "inet/networklayer/common/NetworkInterface.h"

namespace inet {

class INET_API Ieee8022LlcSocketIo : public ApplicationBase, public Ieee8022LlcSocket::ICallback
{
  protected:
    int localSap = -1;
    int remoteSap = -1;
    MacAddress remoteAddress;
    MacAddress localAddress;
    NetworkInterface *networkInterface = nullptr;

    Ieee8022LlcSocket socket;

    int numSent = 0;
    int numReceived = 0;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessageWhenUp(cMessage *message) override;
    virtual void finish() override;
    virtual void refreshDisplay() const override;

    virtual void setSocketOptions();

    virtual void socketDataArrived(Ieee8022LlcSocket *socket, Packet *packet) override;
    virtual void socketClosed(Ieee8022LlcSocket *socket) override;

    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;
};

} // namespace inet

#endif

