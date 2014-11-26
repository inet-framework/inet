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

#ifndef __INET_ROUTINGTABLERECORDER_H
#define __INET_ROUTINGTABLERECORDER_H

#include "inet/common/INETDefs.h"

#if OMNETPP_VERSION >= 0x0500 && defined HAVE_CEVENTLOGLISTENER    /* cEventlogListener is only supported from 5.0 */

#include <map>
#include "inet/networklayer/ipv4/IIPv4RoutingTable.h"
#include "inet/networklayer/common/IRoute.h"

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
    std::map<const InterfaceEntry *, long> interfaceEntryToKey;
    std::map<const IRoute *, long> routeToKey;

  public:
    RoutingTableRecorder();
    virtual ~RoutingTableRecorder();

  protected:
    virtual int numInitStages() const { return NUM_INIT_STAGES; }
    virtual void initialize(int stage);
    virtual void handleMessage(cMessage *);
    virtual void hookListeners();
    virtual void receiveChangeNotification(cModule *source, simsignal_t signalID, cObject *obj);
    virtual void recordSnapshot();
    virtual void recordIndex() {}
    virtual void recordInterface(cModule *host, const InterfaceEntry *ie, simsignal_t signalID);
    virtual void recordRoute(cModule *host, const IRoute *route, simsignal_t signalID);
};

#else /*OMNETPP_VERSION*/

#include "IIPv4RoutingTable.h"

namespace inet {

/**
 * Records routing table changes into a file.
 *
 * @see IPv4RoutingTable, IPv4Route
 */
class INET_API RoutingTableRecorder : public cSimpleModule, public cListener
{
  private:
    FILE *routingLogFile;

  public:
    RoutingTableRecorder();
    virtual ~RoutingTableRecorder();

    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj) { receiveChangeNotification(source, signalID, obj); }

  protected:
    virtual int numInitStages() const { return NUM_INIT_STAGES; }
    virtual void initialize(int stage);
    virtual void handleMessage(cMessage *);
    virtual void hookListeners();
    virtual void ensureRoutingLogFileOpen();
    virtual void receiveChangeNotification(cComponent *source, simsignal_t signalID, cObject *details);
    virtual void recordInterfaceChange(cModule *host, const InterfaceEntry *ie, simsignal_t signalID);
    virtual void recordRouteChange(cModule *host, const IRoute *route, simsignal_t signalID);
};

} // namespace inet

#endif /*OMNETPP_VERSION*/

#endif // ifndef __INET_ROUTINGTABLERECORDER_H

