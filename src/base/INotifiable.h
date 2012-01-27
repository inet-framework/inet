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


#ifndef __INET_INOTIFIABLE_H
#define __INET_INOTIFIABLE_H

#include "INETDefs.h"


/**
 * Clients can receive change notifications from the NotificationBoard via
 * this interface. Clients must "implement" (subclass from) this class.
 *
 * @see NotificationBoard
 * @author Andras Varga
 */
class INET_API INotifiable
{
  public:
    virtual ~INotifiable() {}

    /**
     * Called by the NotificationBoard whenever a change of a category
     * occurs to which this client has subscribed.
     */
    virtual void receiveChangeNotification(int category, const cObject *details) = 0;
};

#endif




