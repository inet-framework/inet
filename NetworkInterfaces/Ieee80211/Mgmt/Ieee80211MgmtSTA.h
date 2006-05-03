//
// Copyright (C) 2006 Andras Varga
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

#ifndef MGMT80211STA_H
#define MGMT80211STA_H

#include <omnetpp.h>
#include "MACAddress.h"
#include "NotificationBoard.h"


/**
 * Used in 802.11 infrastructure mode: handles management frames for a station (STA).
 *
 * @author Andras Varga
 */
class INET_API Mgmt80211STA : public cSimpleModule, public INotifiable
{
  protected:
    virtual int numInitStages() const {return 2;}
    virtual void initialize(int);

    virtual void handleMessage(cMessage *msg);

    /** Called by the NotificationBoard whenever a change occurs we're interested in */
    virtual void receiveChangeNotification(int category, cPolymorphic *details);
};



#endif
