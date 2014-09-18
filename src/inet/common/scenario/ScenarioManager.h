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

#ifndef __INET_SCENARIOMANAGER_H
#define __INET_SCENARIOMANAGER_H

#include "inet/common/INETDefs.h"

#include "inet/common/scenario/IScriptable.h"

namespace inet {

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
    virtual cModule *getRequiredModule(cXMLElement *node, const char *attr);
    virtual cGate *getRequiredGate(cXMLElement *node, const char *modattr, const char *gateattr);
    void createConnection(cXMLElementList& paramList, cChannelType *channelType,
            cGate *srcGate, cGate *destGate);

    // dispatch to command processors
    virtual void processCommand(cXMLElement *node);

    // command processors
    virtual void processAtCommand(cXMLElement *node);
    virtual void processSetParamCommand(cXMLElement *node);
    virtual void processSetChannelAttrCommand(cXMLElement *node);
    virtual void processCreateModuleCommand(cXMLElement *node);
    virtual void processDeleteModuleCommand(cXMLElement *node);
    virtual void processConnectCommand(cXMLElement *node);
    virtual void processDisconnectCommand(cXMLElement *node);
    virtual void processModuleSpecificCommand(cXMLElement *node);

  public:
    ScenarioManager() {}

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual void updateDisplayString();
};

} // namespace inet

#endif // ifndef __INET_SCENARIOMANAGER_H

