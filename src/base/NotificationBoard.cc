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


#include <algorithm>
#include "NotificationBoard.h"
#include "NotifierConsts.h"

Define_Module(NotificationBoard);

std::ostream& operator<<(std::ostream& os, const NotificationBoard::NotifiableVector& v)
{
    os << v.size() << " client(s)";
    for (unsigned int i=0; i<v.size(); i++)
    {
        os << (i==0 ? ": " : ", ");
        if (dynamic_cast<cModule*>(v[i]))
        {
            cModule *mod = dynamic_cast<cModule*>(v[i]);
            os << "mod (" << mod->getClassName() << ")" << mod->getFullName() << " id=" << mod->getId();
        }
        else if (dynamic_cast<cObject*>(v[i]))
        {
            cObject *obj = dynamic_cast<cObject*>(v[i]);
            os << "a " << obj->getClassName();
        }
        else
        {
            os << "a " << opp_typename(typeid(v[i]));
        }
    }
    return os;
}


void NotificationBoard::initialize()
{
    WATCH_MAP(clientMap);
}

void NotificationBoard::handleMessage(cMessage *msg)
{
    error("NotificationBoard doesn't handle messages, it can be accessed via direct method calls");
}


void NotificationBoard::subscribe(INotifiable *client, int category)
{
    Enter_Method("subscribe(%s)", notificationCategoryName(category));

    // find or create entry for this category
    NotifiableVector& clients = clientMap[category];

    // add client if not already there
    if (std::find(clients.begin(), clients.end(), client) == clients.end())
        clients.push_back(client);

    fireChangeNotification(NF_SUBSCRIBERLIST_CHANGED, NULL);
}

void NotificationBoard::unsubscribe(INotifiable *client, int category)
{
    Enter_Method("unsubscribe(%s)", notificationCategoryName(category));

    // find (or create) entry for this category
    NotifiableVector& clients = clientMap[category];

    // remove client if there
    NotifiableVector::iterator it = std::find(clients.begin(), clients.end(), client);
    if (it!=clients.end())
        clients.erase(it);

    fireChangeNotification(NF_SUBSCRIBERLIST_CHANGED, NULL);
}

bool NotificationBoard::hasSubscribers(int category)
{
    ClientMap::iterator it = clientMap.find(category);
    return it!=clientMap.end() && !it->second.empty();
}

void NotificationBoard::fireChangeNotification(int category, const cObject *details)
{
    Enter_Method("fireChangeNotification(%s, %s)", notificationCategoryName(category),
                 details?details->info().c_str() : "n/a");

    ClientMap::iterator it = clientMap.find(category);
    if (it==clientMap.end())
        return;
    NotifiableVector& clients = it->second;
    for (NotifiableVector::iterator j=clients.begin(); j!=clients.end(); j++)
        (*j)->receiveChangeNotification(category, details);
}


