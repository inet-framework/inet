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

#include "inet/common/scenario/ScenarioManager.h"

namespace inet {

Define_Module(ScenarioManager);

void ScenarioManager::initialize()
{
    cXMLElement *script = par("script");

    numChanges = numDone = 0;
    WATCH(numChanges);
    WATCH(numDone);

    for (cXMLElement *node = script->getFirstChild(); node; node = node->getNextSibling()) {
        // check attr t is present
        const char *tAttr = node->getAttribute("t");
        if (!tAttr)
            throw cRuntimeError("attribute 't' missing at %s", node->getSourceLocation());

        // schedule self-message
        simtime_t t = SimTime::parse(tAttr);
        cMessage *msg = new cMessage("scenario-event");
        msg->setContextPointer(node);
        scheduleAt(t, msg);

        // count it
        numChanges++;
    }

    updateDisplayString();
}

void ScenarioManager::handleMessage(cMessage *msg)
{
    cXMLElement *node = (cXMLElement *)msg->getContextPointer();
    delete msg;

    processCommand(node);

    numDone++;
    updateDisplayString();
}

void ScenarioManager::processCommand(cXMLElement *node)
{
    const char *tag = node->getTagName();
    EV << "processing <" << tag << "> command...\n";

    if (!strcmp(tag, "at"))
        processAtCommand(node);
    else if (!strcmp(tag, "set-param"))
        processSetParamCommand(node);
    else if (!strcmp(tag, "set-channel-attr"))
        processSetChannelAttrCommand(node);
    else if (!strcmp(tag, "create-module"))
        processCreateModuleCommand(node);
    else if (!strcmp(tag, "delete-module"))
        processDeleteModuleCommand(node);
    else if (!strcmp(tag, "connect"))
        processConnectCommand(node);
    else if (!strcmp(tag, "disconnect"))
        processDisconnectCommand(node);
    else
        processModuleSpecificCommand(node);
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

const char *ScenarioManager::getRequiredAttribute(cXMLElement *node, const char *attr)
{
    const char *s = node->getAttribute(attr);
    if (!s)
        throw cRuntimeError("required attribute %s of <%s> missing at %s",
                attr, node->getTagName(), node->getSourceLocation());
    return s;
}

cModule *ScenarioManager::getRequiredModule(cXMLElement *node, const char *attr)
{
    const char *moduleAttr = getRequiredAttribute(node, attr);
    cModule *mod = getModuleByPath(moduleAttr);
    if (!mod)
        throw cRuntimeError("module '%s' not found at %s", moduleAttr, node->getSourceLocation());
    return mod;
}

cGate *ScenarioManager::getRequiredGate(cXMLElement *node, const char *modAttr, const char *gateAttr)
{
    cModule *mod = getRequiredModule(node, modAttr);
    const char *gateStr = getRequiredAttribute(node, gateAttr);
    std::string gname;
    int gindex;
    parseIndexedName(gateStr, gname, gindex);
    cGate *g = mod->gate(gname.c_str(), gindex);
    if (!g)
        throw cRuntimeError("module '%s' has no gate '%s' at %s", mod->getFullPath().c_str(), gateStr, node->getSourceLocation());
    return g;
}

void ScenarioManager::processAtCommand(cXMLElement *node)
{
    for (cXMLElement *child = node->getFirstChild(); child; child = child->getNextSibling())
        processCommand(child);
}

void ScenarioManager::processModuleSpecificCommand(cXMLElement *node)
{
    // find which module we'll need to invoke
    cModule *mod = getRequiredModule(node, "module");

    // see if it supports the IScriptable interface
    IScriptable *scriptable = dynamic_cast<IScriptable *>(mod);
    if (!scriptable)
        throw cRuntimeError("<%s> not understood: it is not a built-in command of %s, and module class %s "    //TODO be more specific
                            "is not scriptable (does not subclass from IScriptable) at %s",
                node->getTagName(), getClassName(), mod->getClassName(), node->getSourceLocation());

    // ok, trust it to process this command
    scriptable->processCommand(*node);
}

void ScenarioManager::processSetParamCommand(cXMLElement *node)
{
    // process <set-param> command
    cModule *mod = getRequiredModule(node, "module");
    const char *parAttr = getRequiredAttribute(node, "par");
    const char *valueAttr = getRequiredAttribute(node, "value");

    EV << "Setting " << mod->getFullPath() << "." << parAttr << " = " << valueAttr << "\n";
    bubble((std::string("setting: ") + mod->getFullPath() + "." + parAttr + " = " + valueAttr).c_str());

    // set the parameter to the given value
    cPar& param = mod->par(parAttr);
    param.parse(valueAttr);
}

void ScenarioManager::processSetChannelAttrCommand(cXMLElement *node)
{
    // process <set-channel-attr> command
    cGate *g = getRequiredGate(node, "src-module", "src-gate");
    const char *attrAttr = getRequiredAttribute(node, "attr");
    const char *valueAttr = getRequiredAttribute(node, "value");

    EV << "Setting channel attribute: " << attrAttr << " = " << valueAttr
       << " of gate " << g->getFullPath() << "\n";
    bubble((std::string("setting channel attr: ") + attrAttr + " = " + valueAttr).c_str());

    // make sure gate is connected at all
    if (!g->getNextGate())
        throw cRuntimeError("gate '%s' is not connected at %s", g->getFullPath().c_str(), node->getSourceLocation());

    // find channel (or add one?)
    cChannel *chan = g->getChannel();
    if (!chan)
        throw cRuntimeError("connection starting at gate '%s' has no attributes at %s", g->getFullPath().c_str(), node->getSourceLocation());

    // set the parameter to the given value
    cPar& param = chan->par(attrAttr);
    param.parse(valueAttr);
}

void ScenarioManager::processCreateModuleCommand(cXMLElement *node)
{
    const char *moduleTypeName = getRequiredAttribute(node, "type");
    const char *submoduleName = getRequiredAttribute(node, "submodule");
    const char *parentModulePath = getRequiredAttribute(node, "parent");
    cModuleType *moduleType = cModuleType::get(moduleTypeName);
    if (moduleType == nullptr)
        throw cRuntimeError("module type '%s' is not found", moduleType);
    cModule *parentModule = getSimulation()->getSystemModule()->getModuleByPath(parentModulePath);
    if (parentModule == nullptr)
        throw cRuntimeError("parent module '%s' is not found", parentModulePath);
    cModule *submodule = parentModule->getSubmodule(submoduleName, 0);
    int submoduleIndex = submodule == nullptr ? 0 : submodule->getVectorSize();
    cModule *module = moduleType->create(submoduleName, parentModule, submoduleIndex + 1, submoduleIndex);
    module->finalizeParameters();
    module->buildInside();
    module->callInitialize();
}

void ScenarioManager::processDeleteModuleCommand(cXMLElement *node)
{
    const char *modulePath = getRequiredAttribute(node, "module");
    cModule *module = getSimulation()->getSystemModule()->getModuleByPath(modulePath);
    if (module == nullptr)
        throw cRuntimeError("module '%s' is not found", modulePath);
    module->callFinish();
    module->deleteModule();
}

void ScenarioManager::createConnection(cXMLElementList& paramList, cChannelType *channelType, cGate *srcGate, cGate *destGate)
{
    if (!channelType)
        srcGate->connectTo(destGate);
    else {
        cChannel *channel = channelType->create("channel");

        // set parameters:
        for (auto child : paramList) {
            
            const char *name = getRequiredAttribute(child, "name");
            const char *value = getRequiredAttribute(child, "value");
            channel->par(name).parse(value);
        }

        // connect:
        srcGate->connectTo(destGate, channel);
    }
}

void ScenarioManager::processConnectCommand(cXMLElement *node)
{
    cGate *srcGate;
    cModule *srcMod = getRequiredModule(node, "src-module");
    const char *srcGateStr = getRequiredAttribute(node, "src-gate");
    std::string srcGateName;
    int srcGateIndex;
    parseIndexedName(srcGateStr, srcGateName, srcGateIndex);
    bool isSrcGateInOut = (srcMod->gateType(srcGateName.c_str()) == cGate::INOUT);

    cGate *destGate;
    cModule *destMod = getRequiredModule(node, "dest-module");
    const char *destGateStr = getRequiredAttribute(node, "dest-gate");
    std::string destGateName;
    int destGateIndex;
    parseIndexedName(destGateStr, destGateName, destGateIndex);
    bool isDestGateInOut = (destMod->gateType(destGateName.c_str()) == cGate::INOUT);

    if (srcMod->getParentModule() != destMod->getParentModule())
        throw cRuntimeError("The parent modules of src-module and dest-module are differ at %s",
                node->getSourceLocation());

    // process <connect-channel> command
    const char *channelTypeName = node->getAttribute("channel-type");
    cChannelType *channelType = channelTypeName ? cChannelType::get(channelTypeName) : nullptr;
    cXMLElementList paramList;

    if (channelTypeName)
        paramList = node->getChildrenByTagName("param");

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

void ScenarioManager::processDisconnectCommand(cXMLElement *node)
{
    // process <disconnect> command
    cModule *srcMod = getRequiredModule(node, "src-module");
    cModule *parentMod = srcMod->getParentModule();
    const char *srcGateStr = getRequiredAttribute(node, "src-gate");
    std::string srcGateName;
    int srcGateIndex;
    parseIndexedName(srcGateStr, srcGateName, srcGateIndex);
    cGate::Type srcGateType = srcMod->gateType(srcGateName.c_str());

    cGate *srcGate;

    if (srcGateType == cGate::INPUT)
        throw cRuntimeError("The src-gate must be inout or output gate at %s", node->getSourceLocation());

    if (srcGateType == cGate::INOUT) {
        cGate *g;

        srcGate = srcMod->gateHalf(srcGateName.c_str(), cGate::OUTPUT, srcGateIndex);
        g = srcGate->getNextGate();
        if (!g)
            return; // not connected

        if (g->getOwnerModule()->getParentModule() != parentMod)
            throw cRuntimeError("The src-gate connected to a node on another level at %s", node->getSourceLocation());

        srcGate->disconnect();

        srcGate = srcMod->gateHalf(srcGateName.c_str(), cGate::INPUT, srcGateIndex);
        g = srcGate->getPreviousGate();
        if (!g)
            return; // not connected

        if (g->getOwnerModule()->getParentModule() != parentMod)
            throw cRuntimeError("The src-gate connected to a node on another level at %s", node->getSourceLocation());

        g->disconnect();
    }
    else {
        srcGate = srcMod->gate(srcGateName.c_str(), srcGateIndex);
        cGate *g = srcGate->getNextGate();
        if (g && g->getOwnerModule()->getParentModule() == parentMod)
            srcGate->disconnect();
    }
}

void ScenarioManager::updateDisplayString()
{
    char buf[80];
    sprintf(buf, "total %d changes, %d left", numChanges, numChanges - numDone);
    getDisplayString().setTagArg("t", 0, buf);
}

} // namespace inet

