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

#include "ScenarioManager.h"

Define_Module(ScenarioManager);


void ScenarioManager::initialize()
{
    cXMLElement *script = par("script");

    numChanges = numDone = 0;
    WATCH(numChanges);
    WATCH(numDone);

    for (cXMLElement *node=script->getFirstChild(); node; node = node->getNextSibling())
    {
        // check attr t is present
        const char *tAttr = node->getAttribute("t");
        if (!tAttr)
            error("attribute 't' missing at %s", node->getSourceLocation());

        // schedule self-message
        simtime_t t = STR_SIMTIME(tAttr);
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
    cXMLElement *node = (cXMLElement *) msg->getContextPointer();
    delete msg;

    processCommand(node);

    numDone++;
    updateDisplayString();
}

void ScenarioManager::processCommand(cXMLElement *node)
{
    const char *tag = node->getTagName();
    EV << "processing <" << tag << "> command...\n";

    if (!strcmp(tag,"at"))
        processAtCommand(node);
    else if (!strcmp(tag,"set-param"))
        processSetParamCommand(node);
    else if (!strcmp(tag,"set-channel-attr"))
        processSetChannelAttrCommand(node);
    // else if (!strcmp(tag,"create-module"))
    //    processCreateModuleCommand(node);
    // else if (!strcmp(tag,"connect"))
    //    processConnectCommand(node);
    else
        processModuleSpecificCommand(node);
}

// helper function
static bool parseIndexedName(const char *s, std::string& name, int& index)
{
    const char *b;
    if ((b=strchr(s,'['))==NULL || s[strlen(s)-1]!=']')
    {
        name = s;
        index = -1;
        return false;
    }
    else
    {
        name.assign(s, b-s);
        index = atoi(b+1);
        return true;
    }
}

const char *ScenarioManager::getRequiredAttribute(cXMLElement *node, const char *attr)
{
    const char *s = node->getAttribute(attr);
    if (!s)
        error("required attribute %s of <%s> missing at %s",
              attr, node->getTagName(), node->getSourceLocation());
    return s;
}

cModule *ScenarioManager::getRequiredModule(cXMLElement *node, const char *attr)
{
    const char *moduleAttr = getRequiredAttribute(node, attr);
    cModule *mod = simulation.getModuleByPath(moduleAttr);
    if (!mod)
        error("module '%s' not found at %s", moduleAttr, node->getSourceLocation());
    return mod;
}

cGate *ScenarioManager::getRequiredGate(cXMLElement *node, const char *modAttr, const char *gateAttr)
{
    cModule *mod = getRequiredModule(node, modAttr);
    const char *gateStr = getRequiredAttribute(node, gateAttr);
    std::string gname;
    int gindex;
    cGate *g = parseIndexedName(gateStr, gname, gindex) ? mod->gate(gname.c_str(), gindex) : mod->gate(gname.c_str());
    if (!g)
        error("module '%s' has no gate '%s' at %s", mod->getFullPath().c_str(), gateStr, node->getSourceLocation());
    return g;
}

void ScenarioManager::processAtCommand(cXMLElement *node)
{
    for (cXMLElement *child=node->getFirstChild(); child; child=child->getNextSibling())
        processCommand(child);
}

void ScenarioManager::processModuleSpecificCommand(cXMLElement *node)
{
    // find which module we'll need to invoke
    cModule *mod = getRequiredModule(node, "module");

    // see if it supports the IScriptable interface
    IScriptable *scriptable = dynamic_cast<IScriptable *>(mod);
    if (!scriptable)
        error("<%s> not understood: it is not a built-in command of %s, and module class %s "
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
    bubble((std::string("setting: ")+mod->getFullPath()+"."+parAttr+" = "+valueAttr).c_str());

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
    bubble((std::string("setting channel attr: ")+attrAttr+" = "+valueAttr).c_str());

    // make sure gate is connected at all
    if (!g->getNextGate())
        error("gate '%s' is not connected at %s", g->getFullPath().c_str(), node->getSourceLocation());

    // find channel (or add one?)
    cChannel *chan = g->getChannel();
    if (!chan)
        error("connection starting at gate '%s' has no attributes at %s", g->getFullPath().c_str(), node->getSourceLocation());

    // set the parameter to the given value
    if (!chan->hasPar(attrAttr))
        ; //FIXME remove this "if"
    cPar& param = chan->par(attrAttr);
    param.parse(valueAttr);
}

void ScenarioManager::processCreateModuleCommand(cXMLElement *node)
{
    // FIXME finish and test
}

void ScenarioManager::processDeleteModuleCommand(cXMLElement *node)
{
    // FIXME finish and test
}

void ScenarioManager::processConnectCommand(cXMLElement *node)
{
    // FIXME finish and test
}

void ScenarioManager::processDisconnectCommand(cXMLElement *node)
{
    // FIXME finish and test
}

void ScenarioManager::updateDisplayString()
{
    char buf[80];
    sprintf(buf, "total %d changes, %d left", numChanges, numChanges-numDone);
    getDisplayString().setTagArg("t", 0, buf);
}

