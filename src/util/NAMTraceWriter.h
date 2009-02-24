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

#ifndef __INET_NAMTRACEWRITER_H
#define __INET_NAMTRACEWRITER_H

#include <iostream>
#include <fstream>

#include <omnetpp.h>
#include "INETDefs.h"
#include "INotifiable.h"
#include "NotifierConsts.h"

class NAMTrace;
class InterfaceEntry;

/**
 * Writes a "nam" trace.
 */
class INET_API NAMTraceWriter : public cSimpleModule, public INotifiable
{
  protected:
    int namid;
    NAMTrace *nt;

  protected:
    virtual void recordNodeEvent(const char *state, const char *shape);
    virtual void recordLinkEvent(int peernamid, double datarate, simtime_t delay, const char *state);
    virtual void recordPacketEvent(char event, int peernamid, cPacket *msg);

  protected:
    virtual int numInitStages() const {return 3;}
    virtual void initialize(int stage);
    virtual ~NAMTraceWriter();

    /**
     * Redefined INotifiable method. Called by NotificationBoard on changes.
     */
    virtual void receiveChangeNotification(int category, const cPolymorphic *details);

};

#endif


