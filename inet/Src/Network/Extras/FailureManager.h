//
// Copyright (C) 2005 Vojtech Janota
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

#ifndef FAILUREMANAGER_H
#define FAILUREMANAGER_H

#include <omnetpp.h>

#include "IScriptable.h"

/**
 * TODO documentation
 */
class INET_API FailureManager : public cSimpleModule, public IScriptable
{
  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);

    // IScriptable implementation
    virtual void processCommand(const cXMLElement& node);

  private:
    void replaceNode(cModule *mod, const char *newNodeType);
    void reconnectNode(cModule *old, cModule *n);
    void reconnect(cModule *old, cModule *n, const char *ins, const char *outs);
    cModule* getTargetNode(const char *target);
};

#endif
