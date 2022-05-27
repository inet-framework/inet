//
// Copyright (C) 2005 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/scenario/ScenarioManager.h"

#include "inet/common/INETUtils.h"
#include "inet/common/XMLUtils.h"
#include "inet/common/lifecycle/LifecycleOperation.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/scenario/ScenarioTimer_m.h"

namespace inet {

Define_Module(ScenarioManager);

static const char *ATTR_T = "t";
static const char *ATTR_MODULE = "module";
static const char *ATTR_SRC_MODULE = "src-module";
static const char *ATTR_DEST_MODULE = "dest-module";
static const char *ATTR_SRC_GATE = "src-gate";
static const char *ATTR_DEST_GATE = "dest-gate";
static const char *ATTR_PAR = "par";
static const char *ATTR_NAME = "name";
static const char *ATTR_VALUE = "value";
static const char *ATTR_EXPR = "expr";
static const char *ATTR_TYPE = "type";
static const char *ATTR_VECTOR = "vector";
static const char *ATTR_CHANNEL_TYPE = "channel-type";
static const char *ATTR_SUBMODULE = "submodule";
static const char *ATTR_PARENT = "parent";
static const char *ATTR_OPERATION = "operation";

static const char *CMD_AT = "at";
static const char *CMD_SET_PARAM = "set-param";
static const char *CMD_SET_CHANNEL_PARAM = "set-channel-param";
static const char *CMD_CREATE_MODULE = "create-module";
static const char *CMD_DELETE_MODULE = "delete-module";
static const char *CMD_CONNECT = "connect";
static const char *CMD_DISCONNECT = "disconnect";
static const char *CMD_INITIATE = "initiate";
static const char *TAG_PARAM = "param";

static const char *OP_START = "start";
static const char *OP_STARTUP = "startup";
static const char *OP_STOP = "stop";
static const char *OP_SHUTDOWN = "shutdown";
static const char *OP_CRASH = "crash";
static const char *OP_SUSPEND = "suspend";
static const char *OP_RESUME = "resume";

void ScenarioManager::initialize()
{
    cXMLElement *script = par("script");

    numChanges = numDone = 0;
    WATCH(numChanges);
    WATCH(numDone);

    for (cXMLElement *node = script->getFirstChild(); node; node = node->getNextSibling()) {
        // check attr t is present
        const char *tAttr = node->getAttribute(ATTR_T);
        if (!tAttr)
            throw cRuntimeError("Attribute 't' missing at %s", node->getSourceLocation());

        // schedule self-message
        simtime_t t = SimTime::parse(tAttr);
        auto msg = new ScenarioTimer("scenario-event");
        msg->setXmlNode(node);
        scheduleAt(t, msg);

        // count it
        numChanges++;
    }
}

void ScenarioManager::handleMessage(cMessage *msg)
{
    auto node = check_and_cast<ScenarioTimer *>(msg)->getXmlNode();
    delete msg;

    processCommand(node);

    numDone++;
}

void ScenarioManager::processCommand(const cXMLElement *node)
{
    try {
        std::string tag = node->getTagName();
        EV << "processing <" << tag << "> command...\n";

        if (tag == CMD_AT)
            processAtCommand(node);
        else if (tag == CMD_SET_PARAM)
            processSetParamCommand(node);
        else if (tag == CMD_SET_CHANNEL_PARAM)
            processSetChannelParamCommand(node);
        else if (tag == CMD_CREATE_MODULE)
            processCreateModuleCommand(node);
        else if (tag == CMD_DELETE_MODULE)
            processDeleteModuleCommand(node);
        else if (tag == CMD_CONNECT)
            processConnectCommand(node);
        else if (tag == CMD_DISCONNECT)
            processDisconnectCommand(node);
        else if (tag == CMD_INITIATE || tag == OP_START || tag == OP_STARTUP || tag == OP_STOP
                 || tag == OP_SHUTDOWN || tag == OP_CRASH || tag == OP_SUSPEND || tag == OP_RESUME)
            processLifecycleCommand(node);
        else
            processModuleSpecificCommand(node);
    }
    catch (std::exception& e) {
        throw cRuntimeError("%s, in command <%s> at %s", e.what(), node->getTagName(), node->getSourceLocation());
    }
}

// helper function
static bool parseIndexedName(const char *s, std::string& name, int& index)
{
    const char *b;
    if ((b = strchr(s, '[')) == nullptr || s[strlen(s) - 1] != ']') {
        name = s;
        index = -1;
        return false;
    }
    else {
        name.assign(s, b - s);
        index = atoi(b + 1);
        return true;
    }
}

cModule *ScenarioManager::getRequiredModule(const char *path)
{
    cModule *mod = getModuleByPath(path);
    if (!mod)
        throw cRuntimeError("Module '%s' not found", path);
    return mod;
}

cModule *ScenarioManager::getRequiredModule(const cXMLElement *node, const char *attr)
{
    return getRequiredModule(xmlutils::getMandatoryFilledAttribute(*node, attr));
}

cGate *ScenarioManager::findMandatorySingleGateTowards(cModule *srcModule, cModule *destModule)
{
    cGate *resultGate = nullptr;
    for (cModule::GateIterator it(srcModule); !it.end(); ++it) {
        cGate *gate = *it;
        if (gate->getType() == cGate::OUTPUT && gate->getNextGate() != nullptr && gate->getNextGate()->getOwnerModule() == destModule) {
            if (resultGate)
                throw cRuntimeError("Ambiguous gate: there is more than one connection between the source and destination modules");
            resultGate = gate;
        }
    }
    if (!resultGate)
        throw cRuntimeError("No such gate: there is no connection between the source and destination modules");
    return resultGate;
}

ScenarioManager::GatePair ScenarioManager::getConnection(const cXMLElement *node)
{
    // Connection can be identifier with the source module plus either the source gate or the
    // destination module. The connection must be within a single level of the module hierarchy,
    // i.e. the two modules must be siblings. With the latter option, there must be exactly
    // one connection between the two modules. If the specified gate or connection identifies
    // bidirectional connection (inout gates and '<-->' notation in NED), the source gates
    // of both directions are returned.
    //
    if (node->getAttribute(ATTR_SRC_GATE)) {
        cModule *srcModule = getRequiredModule(node, ATTR_SRC_MODULE);
        const char *srcGateSpec = xmlutils::getMandatoryFilledAttribute(*node, ATTR_SRC_GATE);
        std::string name;
        int index;
        parseIndexedName(srcGateSpec, name, index);
        if (srcModule->gateType(name.c_str()) == cGate::OUTPUT) {
            return GatePair(srcModule->gate(name.c_str(), index), nullptr); // throws if not found
        }
        else if (srcModule->gateType(name.c_str()) == cGate::INOUT) {
            cGate *outputHalf = srcModule->gateHalf(name.c_str(), cGate::OUTPUT, index);
            cGate *inputHalf = srcModule->gateHalf(name.c_str(), cGate::INPUT, index);
            if (outputHalf->getNextGate() == nullptr || inputHalf->getPreviousGate() == nullptr)
                throw cRuntimeError("The specified gate (or the input or output half of it) is not connected");
            if (outputHalf->getNextGate()->getOwnerModule() != inputHalf->getPreviousGate()->getOwnerModule())
                throw cRuntimeError("The specified gate (or the input or output half of it) is connected to a node on another level");
            return GatePair(outputHalf, inputHalf->getPreviousGate());
        }
        else {
            throw cRuntimeError("'src-gate' must be an inout or output gate");
        }
    }
    else if (node->getAttribute(ATTR_DEST_MODULE)) {
        cModule *srcModule = getRequiredModule(node, ATTR_SRC_MODULE);
        cModule *destModule = getRequiredModule(node, ATTR_DEST_MODULE);
        if (srcModule->getParentModule() != destModule->getParentModule())
            throw cRuntimeError("Source and destination modules must be under the same parent");
        cGate *srcGate = findMandatorySingleGateTowards(srcModule, destModule);
        bool bidir = strlen(srcGate->getNameSuffix()) > 0; // TODO use =srcGate->isGateHalf();
        if (!bidir) {
            return GatePair(srcGate, nullptr);
        }
        else {
            cGate *otherHalf = srcModule->gateHalf(srcGate->getBaseName(), cGate::INPUT, srcGate->isVector() ? srcGate->getIndex() : -1); // TODO use =srcGate->getOtherHalf();
            cGate *otherSrcGate = otherHalf->getPreviousGate();
            if (otherSrcGate == nullptr)
                throw cRuntimeError("Broken bidirectional connection: the corresponding input gate is not connected");
            if (otherSrcGate->getOwnerModule() != destModule)
                throw cRuntimeError("Broken bidirectional connection: the input and output gates are connected to two different modules");
            return GatePair(srcGate, otherSrcGate);
        }
    }
    else {
        throw cRuntimeError("Missing attribute: Either 'src-gate' or 'dest-module' must be present");
    }
}

void ScenarioManager::processAtCommand(const cXMLElement *node)
{
    for (const cXMLElement *child = node->getFirstChild(); child; child = child->getNextSibling())
        processCommand(child);
}

void ScenarioManager::processModuleSpecificCommand(const cXMLElement *node)
{
    // find which module we'll need to invoke
    cModule *mod = getRequiredModule(node, ATTR_MODULE);

    // see if it supports the IScriptable interface
    IScriptable *scriptable = dynamic_cast<IScriptable *>(mod);
    if (!scriptable)
        throw cRuntimeError("<%s> not understood: it is not a built-in command of %s, and module class %s "
                            "is not scriptable (does not subclass from IScriptable)",
                node->getTagName(), getClassName(), mod->getClassName());

    // ok, trust it to process this command
    scriptable->processCommand(*node);
}

void ScenarioManager::setParamFromXml(cPar& param, const cXMLElement *node)
{
    const char *valueAttr = node->getAttribute(ATTR_VALUE);
    const char *exprAttr = node->getAttribute(ATTR_EXPR);
    if (!valueAttr && !exprAttr)
        throw cRuntimeError("<%s>: required any '%s' or '%s' attribute", node->getTagName(), ATTR_VALUE, ATTR_EXPR);
    if (valueAttr && exprAttr)
        throw cRuntimeError("<%s>: required only one from '%s' and '%s' attributes", node->getTagName(), ATTR_VALUE, ATTR_EXPR);
    if (exprAttr)
        param.parse(exprAttr);
    else {
        switch (param.getType()) {
            case cPar::STRING: param.setStringValue(valueAttr); break;
            case cPar::XML:    param.setXMLValue(getEnvir()->getParsedXMLString(valueAttr)); break;
            default:           param.parse(valueAttr); break;
        }
    }
}

void ScenarioManager::processSetParamCommand(const cXMLElement *node)
{
    // process <set-param> command
    cModule *mod = getRequiredModule(node, ATTR_MODULE);
    const char *parAttr = xmlutils::getMandatoryFilledAttribute(*node, ATTR_PAR);
    cPar& param = mod->par(parAttr);
    setParamFromXml(param, node);
    EV << "Setting " << param.getFullPath() << " = " << param.str() << "\n";
    bubble(("setting: " + param.getFullPath() + " = " + param.str()).c_str());
}

cPar& ScenarioManager::getChannelParam(cGate *srcGate, const char *name)
{
    // make sure gate is connected at all
    if (!srcGate->getNextGate())
        throw cRuntimeError("Gate '%s' is not connected", srcGate->getFullPath().c_str());

    // get channel object
    cChannel *chan = srcGate->getChannel();
    if (!chan)
        throw cRuntimeError("Connection starting at gate '%s' has no channel object", srcGate->getFullPath().c_str());

    // set the parameter to the given value
    cPar& param = chan->par(name);
    return param;
}

void ScenarioManager::processSetChannelParamCommand(const cXMLElement *node)
{
    // process <set-channel-param> command
    GatePair pair = getConnection(node);
    const char *parAttr = xmlutils::getMandatoryFilledAttribute(*node, ATTR_PAR);
    const char *valueAttr = xmlutils::getMandatoryAttribute(*node, ATTR_VALUE);

    cPar& param = getChannelParam(pair.first, parAttr);
    setParamFromXml(param, node);
    if (pair.second)
        setParamFromXml(getChannelParam(pair.second, parAttr), node);

    EV << "Setting channel parameter: " << parAttr << " = " << valueAttr
       << " on connection " << pair.first->getFullPath() << " --> " << pair.first->getNextGate()->getFullPath()
       << (pair.second ? " and its reverse connection" : "") << "\n";
    bubble((std::string("setting channel parameter: ") + parAttr + " = " + param.str()).c_str());
}

void ScenarioManager::processCreateModuleCommand(const cXMLElement *node)
{
    const char *moduleTypeName = xmlutils::getMandatoryFilledAttribute(*node, ATTR_TYPE);
    const char *submoduleName = xmlutils::getMandatoryFilledAttribute(*node, ATTR_SUBMODULE);
    const char *parentModulePath = xmlutils::getMandatoryFilledAttribute(*node, ATTR_PARENT);
    cModuleType *moduleType = cModuleType::get(moduleTypeName);
    cModule *parentModule = getSimulation()->getSystemModule()->getModuleByPath(parentModulePath);
    if (parentModule == nullptr)
        throw cRuntimeError("Parent module '%s' is not found", parentModulePath);

    // TODO solution for inconsistent out-of-date vectorSize values in OMNeT++
    int submoduleVectorSize = 0;
    for (SubmoduleIterator it(parentModule); !it.end(); ++it) {
        cModule *submodule = *it;
        if (submodule->isVector() && submodule->isName(submoduleName)) {
            if (submoduleVectorSize < submodule->getVectorSize())
                submoduleVectorSize = submodule->getVectorSize();
        }
    }

    bool vector = xmlutils::getAttributeBoolValue(node, ATTR_VECTOR, submoduleVectorSize > 0);
    cModule *module = nullptr;
    if (vector) {
        if (!parentModule->hasSubmoduleVector(submoduleName)) {
            parentModule->addSubmoduleVector(submoduleName, submoduleVectorSize + 1);
        }
        else
            parentModule->setSubmoduleVectorSize(submoduleName, submoduleVectorSize + 1);
        module = moduleType->create(submoduleName, parentModule, submoduleVectorSize);
    }
    else {
        module = moduleType->create(submoduleName, parentModule);
    }
    module->finalizeParameters();
    module->buildInside();
    cPreModuleInitNotification pre;
    pre.module = module;
    emit(POST_MODEL_CHANGE, &pre);
    module->callInitialize();
    cPostModuleInitNotification post;
    post.module = module;
    emit(POST_MODEL_CHANGE, &post);
}

void ScenarioManager::processDeleteModuleCommand(const cXMLElement *node)
{
    const char *modulePath = xmlutils::getMandatoryFilledAttribute(*node, ATTR_MODULE);
    cModule *module = getSimulation()->getSystemModule()->getModuleByPath(modulePath);
    if (module == nullptr)
        throw cRuntimeError("Module '%s' not found", modulePath);
    module->callFinish();
    module->deleteModule();
}

void ScenarioManager::createConnection(const cXMLElementList& paramList, cChannelType *channelType, cGate *srcGate, cGate *destGate)
{
    if (!channelType)
        srcGate->connectTo(destGate);
    else {
        cChannel *channel = channelType->create("channel");

        // set parameters:
        for (auto child : paramList) {
            const char *name = xmlutils::getMandatoryFilledAttribute(*child, ATTR_NAME);
            const char *value = xmlutils::getMandatoryAttribute(*child, ATTR_VALUE);
            channel->par(name).parse(value);
        }

        // connect:
        srcGate->connectTo(destGate, channel);
    }
}

void ScenarioManager::processConnectCommand(const cXMLElement *node)
{
    cGate *srcGate;
    cModule *srcMod = getRequiredModule(node, ATTR_SRC_MODULE);
    const char *srcGateStr = xmlutils::getMandatoryFilledAttribute(*node, ATTR_SRC_GATE);
    std::string srcGateName;
    int srcGateIndex;
    parseIndexedName(srcGateStr, srcGateName, srcGateIndex);
    bool isSrcGateInOut = (srcMod->gateType(srcGateName.c_str()) == cGate::INOUT);

    cGate *destGate;
    cModule *destMod = getRequiredModule(node, ATTR_DEST_MODULE);
    const char *destGateStr = xmlutils::getMandatoryFilledAttribute(*node, ATTR_DEST_GATE);
    std::string destGateName;
    int destGateIndex;
    parseIndexedName(destGateStr, destGateName, destGateIndex);
    bool isDestGateInOut = (destMod->gateType(destGateName.c_str()) == cGate::INOUT);

    if (srcMod->getParentModule() != destMod->getParentModule())
        throw cRuntimeError("'src-module' and 'dest-module' must have the same parent module");

    // process <connect-channel> command
    const char *channelTypeName = node->getAttribute(ATTR_CHANNEL_TYPE);
    cChannelType *channelType = channelTypeName ? cChannelType::get(channelTypeName) : nullptr;
    cXMLElementList paramList;

    if (channelTypeName)
        paramList = node->getChildrenByTagName(TAG_PARAM);

    srcGate = isSrcGateInOut ?
        srcMod->gateHalf(srcGateName.c_str(), cGate::OUTPUT, srcGateIndex) :
        srcMod->gate(srcGateName.c_str(), srcGateIndex);
    destGate = isDestGateInOut ?
        destMod->gateHalf(destGateName.c_str(), cGate::INPUT, destGateIndex) :
        destMod->gate(destGateName.c_str(), destGateIndex);

    createConnection(paramList, channelType, srcGate, destGate);

    if (isSrcGateInOut && isDestGateInOut) {
        destGate = srcMod->gateHalf(srcGateName.c_str(), cGate::INPUT, srcGateIndex);
        srcGate = destMod->gateHalf(destGateName.c_str(), cGate::OUTPUT, destGateIndex);

        createConnection(paramList, channelType, srcGate, destGate);
    }
}

void ScenarioManager::disconnect(cGate *srcGate)
{
    srcGate->disconnect();
}

void ScenarioManager::processDisconnectCommand(const cXMLElement *node)
{
    // process <disconnect> command
    GatePair pair = getConnection(node);

    EV << "Disconnecting " << pair.first->getFullPath() << " --> " << pair.first->getNextGate()->getFullPath()
       << (pair.second ? " and its reverse connection" : "") << "\n";

    disconnect(pair.first);
    if (pair.second)
        disconnect(pair.second);
}

void ScenarioManager::processLifecycleCommand(const cXMLElement *node)
{
    // resolve target module
    const char *target = node->getAttribute(ATTR_MODULE);
    cModule *module = getModuleByPath(target);
    if (!module)
        throw cRuntimeError("Module '%s' not found", target);

    // resolve operation
    std::string tag = node->getTagName();
    std::string operationName = (tag == CMD_INITIATE) ? node->getAttribute(ATTR_OPERATION) : tag;
    LifecycleOperation *operation;
    if (operationName == OP_START || operationName == OP_STARTUP)
        operation = new ModuleStartOperation;
    else if (operationName == OP_STOP || operationName == OP_SHUTDOWN)
        operation = new ModuleStopOperation;
    else if (operationName == OP_CRASH)
        operation = new ModuleCrashOperation;
    else
        operation = check_and_cast<LifecycleOperation *>(inet::utils::createOne(operationName.c_str()));

    auto paramsCopy = node->getAttributes();
    paramsCopy.erase(ATTR_MODULE);
    paramsCopy.erase(ATTR_T);
    paramsCopy.erase(ATTR_OPERATION);
    operation->initialize(module, paramsCopy);
    if (!paramsCopy.empty())
        throw cRuntimeError("Unknown parameter '%s' for operation %s", paramsCopy.begin()->first.c_str(), operationName.c_str());

    // do the operation
    initiateOperation(operation);
}

void ScenarioManager::refreshDisplay() const
{
    char buf[80];
    sprintf(buf, "total %d changes, %d left", numChanges, numChanges - numDone);
    getDisplayString().setTagArg(ATTR_T, 0, buf);
}

} // namespace inet

