//
// Copyright (C) 2005 Vojtech Janota
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

#ifndef FAILUREMANAGER_H
#define FAILUREMANAGER_H

#include "INETDefs.h"

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

  protected:
    virtual void replaceNode(cModule *mod, const char *newNodeType);
    virtual void reconnectNode(cModule *old, cModule *n);
    virtual void reconnectAllGates(cModule *old, cModule *n);
    virtual void reconnectGates(cModule *old, cModule *n, const char *gateName, int gateIndex = -1);
    virtual void reconnectGate(cGate *oldGate, cGate *newGate);
    virtual cModule* getTargetNode(const char *target);
  private:
    static cChannel *copyChannel(cChannel *channel);
    static void copyParams(cComponent *from, cComponent *to);
};

#endif
