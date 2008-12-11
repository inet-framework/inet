//
// Copyright (C) 2005 Andras Varga
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


#ifndef __INET_NOTIFICATIONBOARD_H
#define __INET_NOTIFICATIONBOARD_H

#include <omnetpp.h>
#include <map>
#include <vector>
#include "ModuleAccess.h"
#include "INotifiable.h"
#include "NotifierConsts.h"

/**
 * Acts as a intermediary between module where state changes can occur and
 * modules which are interested in learning about those changes;
 * "Notification Broker".
 *
 * Notification events are grouped into "categories." Examples of categories
 * are: NF_HOSTPOSITION_UPDATED, NF_RADIOSTATE_CHANGED, NF_PP_TX_BEGIN,
 * NF_PP_TX_END, NF_IPv4_ROUTE_ADDED, NF_BEACON_LOST
 * NF_NODE_FAILURE, NF_NODE_RECOVERY, etc. Each category is identified by
 * an integer (right now it's assigned in the source code via an enum,
 * in the future we'll convert to dynamic category registration).
 *
 * To trigger a notification, the client must obtain a pointer to the
 * NotificationBoard of the given host or router (explained later), and
 * call its fireChangeNotification() method. The notification will be
 * delivered to all subscribed clients immediately, inside the
 * fireChangeNotification() call.
 *
 * Clients that wish to receive notifications should implement (subclass from)
 * the INotifiable interface, obtain a pointer to the NotificationBoard,
 * and subscribe to the categories they are interested in by calling the
 * subscribe() method of the NotificationBoard. Notifications will be
 * delivered to the receiveChangeNotification() method of the client
 * (redefined from INotifiable).
 *
 * In cases when the category itself (an int) does not carry enough information
 * about the notification event, one can pass additional information
 * in a data class. There is no restriction on what the data class may contain,
 * except that it has to be subclassed from cPolymorphic, and of course
 * producers and consumers of notifications should agree on its contents.
 * If no extra info is needed, one can pass a NULL pointer in the
 * fireChangeNotification() method.
 *
 * A module which implements INotifiable looks like this:
 *
 * <pre>
 * class Foo : public cSimpleModule, public INotifiable {
 *     ...
 *     virtual void receiveChangeNotification(int category, const cPolymorphic *details) {..}
 *     ...
 * };
 * </pre>
 *
 * Obtaining a pointer to the NotificationBoard module of that host/router:
 *
 * <pre>
 * NotificationBoard *nb; // this is best made a module class member
 * nb = NotificationBoardAccess().get();  // best done in initialize()
 * </pre>
 *
 *
 * See NED file for additional info.
 *
 * @see INotifiable
 * @author Andras Varga
 */
class INET_API NotificationBoard : public cSimpleModule
{
  public: // should be protected
    typedef std::vector<INotifiable *> NotifiableVector;
    typedef std::map<int, NotifiableVector> ClientMap;
    friend std::ostream& operator<<(std::ostream&, const NotifiableVector&); // doesn't work in MSVC 6.0

  protected:
    ClientMap clientMap;

  protected:
    /**
     * Initialize.
     */
    virtual void initialize();

    /**
     * Does nothing.
     */
    virtual void handleMessage(cMessage *msg);

  public:
    /** @name Methods for consumers of change notifications */
    //@{
    /**
     * Subscribe to changes of the given category
     */
    virtual void subscribe(INotifiable *client, int category);

    /**
     * Unsubscribe from changes of the given category
     */
    virtual void unsubscribe(INotifiable *client, int category);

    /**
     * Returns true if any client has subscribed to the given category.
     * This, by using a local boolean 'hasSubscriber' flag, allows
     * performance-critical clients to leave out calls to
     * fireChangeNotification() if there's no one subscribed anyway.
     * The flag should be refreshed on each NF_SUBSCRIBERLIST_CHANGED
     * notification.
     */
    virtual bool hasSubscribers(int category);
    //@}

    /** @name Methods for producers of change notifications */
    //@{
    /**
     * Tells NotificationBoard that a change of the given category has
     * taken place. The optional details object may carry more specific
     * information about the change (e.g. exact location, specific attribute
     * that changed, old value, new value, etc).
     */
    virtual void fireChangeNotification(int category, const cPolymorphic *details=NULL);
    //@}
};

/**
 * Gives access to the NotificationBoard instance within the host/router.
 */
class INET_API NotificationBoardAccess : public ModuleAccess<NotificationBoard>
{
  public:
    NotificationBoardAccess() : ModuleAccess<NotificationBoard>("notificationBoard") {}
};

#endif




