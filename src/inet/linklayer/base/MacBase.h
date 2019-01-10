//
// Copyright (C) 2013 Opensim Ltd.
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
// Author: Andras Varga (andras@omnetpp.org)
//

#ifndef __INET_MACBASE_H
#define __INET_MACBASE_H

#include "inet/common/INETDefs.h"
#include "inet/linklayer/base/MacProtocolBase.h"
#include "inet/linklayer/common/MacAddress.h"

namespace inet {

class InterfaceEntry;

/**
 * Base class for MAC modules.
 */
class INET_API MacBase : public MacProtocolBase
{
  protected:
    cModule *hostModule = nullptr;

  public:
    MacBase() {}
    virtual ~MacBase();

  protected:
    using cListener::receiveSignal;
    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void handleMessageWhenDown(cMessage *msg) override;
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details) override;
    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;

    /**
     * should clear queue and emit signal "packetDropped" with entire packets
     */
    virtual void flushQueue() = 0;

    /**
     * should clear queue silently
     */
    virtual void clearQueue() = 0;
};

} // namespace inet

#endif // ifndef __INET_MACBASE_H

