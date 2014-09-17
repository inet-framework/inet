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
#include "inet/common/lifecycle/ILifecycle.h"

namespace inet {

class InterfaceEntry;

/**
 * Base class for MAC modules.
 */
class INET_API MACBase : public cSimpleModule, public ILifecycle, public cListener
{
  protected:
    cModule *hostModule;
    bool isOperational;    // for use in handleMessage()
    InterfaceEntry *interfaceEntry;    // NULL if no InterfaceTable or node is down

  public:
    MACBase();
    virtual ~MACBase();

  protected:
    using cListener::receiveSignal;
    virtual void initialize(int stage);
    virtual int numInitStages() const { return NUM_INIT_STAGES; }
    void registerInterface();    // do not override! override createInterfaceEntry()
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj);
    virtual bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback);
    virtual void updateOperationalFlag(bool isNodeUp);
    virtual bool isNodeUp();
    virtual void handleMessageWhenDown(cMessage *msg);

    /**
     * should create InterfaceEntry
     */
    virtual InterfaceEntry *createInterfaceEntry() = 0;

    /**
     * should clear queue and emit signal "dropPkFromHLIfaceDown" with entire packets
     */
    virtual void flushQueue() = 0;

    /**
     * should clear queue silently
     */
    virtual void clearQueue() = 0;

    /**
     * should return true if the msg arrived from upper layer, else return false
     */
    virtual bool isUpperMsg(cMessage *msg) = 0;
};

} // namespace inet

#endif // ifndef __INET_MACBASE_H

