//
// Copyright (C) 2012 Andras Varga
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

#ifndef __ROUTINGTABLERECORDER_H
#define __ROUTINGTABLERECORDER_H

#include "INETDefs.h"

#if OMNETPP_VERSION >= 0x0500 && defined HAVE_CEVENTLOGLISTENER  /* cEventlogListener is only supported from 5.0 */

#include <map>
#include "IIPv4RoutingTable.h"
#include "INotifiable.h"
#include "IRoute.h"

/**
 * Records interface table and routing table changes into the eventlog.
  *
 * @see IPv4RoutingTable, IPv4Route
 */
class INET_API RoutingTableRecorder : public cSimpleModule, public cIndexedEventlogManager::cEventlogListener
{
    friend class RoutingTableNotificationBoardListener;

  protected:
    struct EventLogEntryReference
    {
        eventnumber_t eventNumber;
        int entryIndex;

        EventLogEntryReference()
        {
            this->eventNumber = -1;
            this->entryIndex = -1;
        }

        EventLogEntryReference(eventnumber_t eventNumber, int entryIndex)
        {
            this->eventNumber = eventNumber;
            this->entryIndex = entryIndex;
        }
    };

    long interfaceKey;
    long routeKey;
    std::map<InterfaceEntry *, long> interfaceEntryToKey;
    std::map<IRoute *, long> routeToKey;

  public:
    RoutingTableRecorder();
    virtual ~RoutingTableRecorder();

  protected:
    virtual int numInitStages() const  {return 1;}
    virtual void initialize(int stage);
    virtual void handleMessage(cMessage *);
    virtual void hookListeners();
    virtual void receiveChangeNotification(NotificationBoard *nb, int category, const cObject *details);
    virtual void recordSnapshot();
    virtual void recordIndex() {}
    virtual void recordInterface(cModule *host, InterfaceEntry *ie, int category);
    virtual void recordRoute(cModule *host, IRoute *route, int category);
};

#endif /*OMNETPP_VERSION*/

#endif
