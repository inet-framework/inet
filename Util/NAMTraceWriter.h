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

#ifndef __NAMTRACEWRITER_H
#define __NAMTRACEWRITER_H

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
    void recordNodeEvent(char *state, char *shape);
    void recordLinkEvent(int peernamid, double datarate, double delay, char *state);
    void recordLinkEvent(InterfaceEntry *ie, char *state);
    void recordPacketEvent(const char event, int peernamid, cMessage *msg);

  protected:
    virtual int numInitStages() const {return 3;}
    virtual void initialize(int stage);
    virtual void finish();

    /**
     * Redefined INotifiable method. Called by NotificationBoard on changes.
     */
    virtual void receiveChangeNotification(int category, cPolymorphic *details);

};

#endif


