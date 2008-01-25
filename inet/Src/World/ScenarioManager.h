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

#ifndef SCENARIOMANAGER_H
#define SCENARIOMANAGER_H

#include <omnetpp.h>
#include "INETDefs.h"
#include "IScriptable.h"


/**
 * Scenario Manager (experimental) which executes a script specified in XML.
 * ScenarioManager has a few built-in commands such as \<set-param>,
 * \<set-channel-attr>, etc, and can pass commands to modules that implement
 * the IScriptable interface. The \<at> built-in command can be used to
 * group commands to be carried out at the same simulation time.
 *
 * See NED file for details.
 *
 * @see IScriptable
 * @author Andras Varga
 */
class INET_API ScenarioManager : public cSimpleModule
{
  protected:
    // total number of changes, and number of changes already done
    int numChanges;
    int numDone;

  protected:
    // utilities
    const char *getRequiredAttribute(cXMLElement *node, const char *attr);
    cModule *getRequiredModule(cXMLElement *node, const char *attr);
    cGate *getRequiredGate(cXMLElement *node, const char *modattr, const char *gateattr);

    // dispatch to command processors
    void processCommand(cXMLElement *node);

    // command processors
    void processAtCommand(cXMLElement *node);
    void processSetParamCommand(cXMLElement *node);
    void processSetChannelAttrCommand(cXMLElement *node);
    void processCreateModuleCommand(cXMLElement *node);
    void processDeleteModuleCommand(cXMLElement *node);
    void processConnectCommand(cXMLElement *node);
    void processDisconnectCommand(cXMLElement *node);
    void processModuleSpecificCommand(cXMLElement *node);

  public:
    ScenarioManager() {}

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    void updateDisplayString();
};

#endif
