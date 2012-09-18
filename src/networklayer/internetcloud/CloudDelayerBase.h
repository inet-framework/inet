//
// Copyright (C) 2012 OpenSim Ltd
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
// @author Zoltan Bojthe
//

#ifndef __INET_INTERNETCLOUD_CLOUDDELAYERBASE_H
#define __INET_INTERNETCLOUD_CLOUDDELAYERBASE_H


#include "INETDefs.h"

class INET_API CloudDelayerBase : public cSimpleModule
{
  protected:
    virtual void handleMessage(cMessage *msg);

    /**
     * Returns true in outDrop if the msg is dropped in cloud,
     * otherwise returns calculated delay in outDelay.
     */
    virtual void calculateDropAndDelay(const cMessage *msg, int srcID, int destID, bool& outDrop, simtime_t& outDelay);
};

#endif  // __INET_INTERNETCLOUD_CLOUDDELAYERBASE_H

