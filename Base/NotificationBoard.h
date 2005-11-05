//
// Copyright (C) 2005 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//


#ifndef __NOTIFIER_H
#define __NOTIFIER_H

#ifdef _MSC_VER
#pragma warning(disable : 4786)
#endif

#include <omnetpp.h>
#include <map>
#include <vector>
#include "ModuleAccess.h"
#include "INotifiable.h"


/**
 * Acts as a intermediary between module where state changes can occur and
 * modules which are interested in learning about those changes;
 * "Notification Broker".
 *
 * Modules that wish to receive notifications should implement the
 * INotifiable interface.
 *
 * Notification categories should be layed out like this:
 *    - layer 1 (physical): 1000-1999
 *    - layer 2 (data-link): 2000-2999
 *    - layer 3 (network): 3000-3999
 *    - layer 4 (transport): 4000-4999
 *    - layer 7 (application): 7000-7999
 *    - mobility: 8000-8999
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
    void subscribe(INotifiable *client, int category);

    /**
     * Unsubscribe from changes of the given category
     */
    void unsubscribe(INotifiable *client, int category);
    //@}

    /** @name Methods for producers of change notifications */
    //@{
    /**
     * Tells NotificationBoard that a change of the given category has
     * taken place. The optional details object may carry more specific
     * information about the change (e.g. exact location, specific attribute
     * that changed, old value, new value, etc).
     */
    void fireChangeNotification(int category, cPolymorphic *details=NULL);
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




