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

#ifndef CHANNELINSTALLER_H
#define CHANNELINSTALLER_H

#include <omnetpp.h>
#include "INETDefs.h"


/**
 * Replaces channel objects in the network.
 *
 * This module is a temporary solution until the NED infrastructure
 * gets extended to accomodate channel classes.
 *
 * @author Andras Varga
 */
class INET_API ChannelInstaller : public cSimpleModule
{
  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual cChannel *createReplacementChannelFor(cChannel *channel);
};

#endif
