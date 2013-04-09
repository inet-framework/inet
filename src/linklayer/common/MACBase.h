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

#ifndef __INET_MACBASE_H_
#define __INET_MACBASE_H_

#include "INETDefs.h"
#include "ILifecycle.h"
#include "INotifiable.h"

class NotificationBoard;
class InterfaceEntry;

/**
 * Base class for MAC modules.
 */
class INET_API MACBase : public cSimpleModule, public ILifecycle, public INotifiable
{
    protected:
        //TODO make private + add getter
        NotificationBoard *nb;
        bool isOperational;  // for use in handleMessage()
        InterfaceEntry *interfaceEntry;  // NULL if no InterfaceTable or node is down

    public:
        MACBase();
        virtual ~MACBase();

    protected:
        virtual void initialize(int stage);
        virtual void registerInterface(); // do not override! override createInterfaceEntry()
        virtual InterfaceEntry *createInterfaceEntry() = 0;
        virtual void receiveChangeNotification(int category, const cObject *details);
        virtual bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback);
        virtual void updateOperationalFlag(bool isNodeUp);
        virtual bool isNodeUp();
        virtual void flushQueue() = 0;
        virtual void handleMessageWhenDown(cMessage *msg);
        virtual bool isUpperMsg(cMessage *msg) = 0;
};

#endif
