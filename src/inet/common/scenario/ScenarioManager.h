//
// Copyright (C) 2005 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_SCENARIOMANAGER_H
#define __INET_SCENARIOMANAGER_H

#include "inet/common/lifecycle/LifecycleController.h"
#include "inet/common/scenario/IScriptable.h"

namespace inet {

// TODO replace these with standard omnet notifications when they become available
class INET_API cPreModuleInitNotification : public cModelChangeNotification
{
  public:
    cModule *module;
};

// TODO replace these with standard omnet notifications when they become available
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
 */
class INET_API ScenarioManager : public cSimpleModule, public LifecycleController
{
  protected:
    // total number of changes, and number of changes already done
    int numChanges = 0;
    int numDone = 0;

  protected:
    // utilities
    typedef std::pair<cGate *, cGate *> GatePair;
    cModule *getRequiredModule(const char *path);
    cModule *getRequiredModule(const cXMLElement *node, const char *attr);
    cGate *findMandatorySingleGateTowards(cModule *srcModule, cModule *destModule);
    GatePair getConnection(const cXMLElement *node);
    void setParamFromXml(cPar& param, const cXMLElement *node);
    cPar& getChannelParam(cGate *srcGate, const char *name);
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

#endif

