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
#include "inet/common/lifecycle/LifecycleController.h"
#include "inet/common/scenario/IScriptable.h"

namespace inet {

// TODO: replace these with standard omnet notifications when they become available
class INET_API cPreModuleInitNotification : public cModelChangeNotification
{
  public:
    cModule *module;
};

// TODO: replace these with standard omnet notifications when they become available
class INET_API cPostModuleInitNotification : public cModelChangeNotification
{
  public:
    cModule *module;
};

/**
 * Scenario Manager (experimental) which executes a script specified in XML.
 * ScenarioManager has a few built-in commands such as \<set-param>,
 * \<set-channel-param>, etc, and can pass commands to modules that implement
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
    int numChanges = 0;
    int numDone = 0;

    LifecycleController lifecycleController;

  protected:
    // utilities
    typedef std::pair<cGate*,cGate*> GatePair;
    cModule *getRequiredModule(const char *path);
    cModule *getRequiredModule(const cXMLElement *node, const char *attr);
    cGate *findMandatorySingleGateTowards(cModule *srcModule, cModule *destModule);
    GatePair getConnection(const cXMLElement *node);
    void setChannelParam(cGate *srcGate, const char *name, const char *value);
    void disconnect(cGate *srcGate);
    void createConnection(const cXMLElementList& paramList, cChannelType *channelType, cGate *srcGate, cGate *destGate);

    // dispatch to command processors
    virtual void processCommand(const cXMLElement *node);

    // command processors
    virtual void processAtCommand(const cXMLElement *node);
    virtual void processSetParamCommand(const cXMLElement *node);
    virtual void processSetChannelParamCommand(const cXMLElement *node);
    virtual void processCreateModuleCommand(const cXMLElement *node);
    virtual void processDeleteModuleCommand(const cXMLElement *node);
    virtual void processConnectCommand(const cXMLElement *node);
    virtual void processDisconnectCommand(const cXMLElement *node);
    virtual void processModuleSpecificCommand(const cXMLElement *node);
    virtual void processLifecycleCommand(const cXMLElement *node);

  public:
    ScenarioManager() {}

  protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void refreshDisplay() const override;
};

} // namespace inet

#endif // ifndef __INET_SCENARIOMANAGER_H

