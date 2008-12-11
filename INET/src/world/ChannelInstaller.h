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
