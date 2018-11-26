//
// Copyright (C) 2004 Andras Varga
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
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

#ifndef __INET_IPVXTRAFSINK_H
#define __INET_IPVXTRAFSINK_H

#include <vector>

#include "inet/applications/base/ApplicationBase.h"
#include "inet/common/INETDefs.h"
#include "inet/common/packet/Packet.h"
#include "inet/networklayer/common/L3Address.h"

namespace inet {

/**
 * Consumes and prints packets received from the IP module. See NED for more info.
 */
class INET_API IpvxTrafSink : public ApplicationBase
{
  protected:
    int numReceived;

  protected:
    virtual void printPacket(Packet *msg);
    virtual void processPacket(Packet *msg);

    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void handleMessageWhenUp(cMessage *msg) override;
    virtual void refreshDisplay() const override;

    virtual void handleStartOperation(LifecycleOperation *operation) override { }
    virtual void handleStopOperation(LifecycleOperation *operation) override { }
    virtual void handleCrashOperation(LifecycleOperation *operation) override { }
};

} // namespace inet

#endif // ifndef __INET_IPVXTRAFSINK_H

