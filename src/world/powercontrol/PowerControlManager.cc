//
// Copyrigth 2010 Juan-Carlos Maureira
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#include "IPowerControl.h"
#include "PowerControlManager.h"
#include "PowerControlMessages_m.h"

Define_Module(PowerControlManager);

PowerControlManager* PowerControlManager::get() {
    cModule* mod = simulation.getModuleByPath("powerControlManager");
    if (check_and_cast<PowerControlManager*>(mod)) {
         return check_and_cast<PowerControlManager*>(mod);
    }
    return NULL;
}

void PowerControlManager::initialize()
{
	EV << "Power Control Initialization" << endl;
	// schedule a power control event for testing

	//int run_id = simulation.getActiveEnvir()->getConfigEx()->getActiveRunNumber();a

    int repetition_id = atoi(simulation.getActiveEnvir()->getConfigEx()->getVariable("repetition"));

    EV << "PowerControlMananger: repetition_id: " << repetition_id << endl;

    cXMLElement* events_definition = par("powerControlEvents").xmlValue();

    if (events_definition!=NULL) {
    	// process enable events
        cXMLElementList enable_events = events_definition->getChildrenByTagName("Enable");
        for(cXMLElementList::iterator it = enable_events.begin();it!=enable_events.end();it++)
        {
            cXMLElement* event = (*it);
            if (event->getAttribute("module")!=NULL && event->getAttribute("when")!=NULL && event->getAttribute("eventId")!=NULL)
            {
                std::string module_name = event->getAttribute("module");
                simtime_t when = atof(event->getAttribute("when"));
                std::string id_str = event->getAttribute("eventId");
                if (id_str!="")
                {
                    int id_event = atoi(id_str.c_str());
                    if (repetition_id == id_event)
                    {
                        // this event is meant for this run id. schedule it
                        this->scheduleEnablePowerControlEvent(when,module_name);
                    }
                }
                else
                {
                    // no run id provided. schedule the event
                    this->scheduleEnablePowerControlEvent(when,module_name);
                }
            }
            else
            {
                error("PowerControlEvent definition does not specify module, when or eventId: %s",event->tostr(1).c_str());
            }
        }

        // process disable events
        cXMLElementList disable_events = events_definition->getChildrenByTagName("Disable");
        for(cXMLElementList::iterator it = disable_events.begin();it!=disable_events.end();it++)
        {
            cXMLElement* event = (*it);
            if (event->getAttribute("module")!=NULL && event->getAttribute("when")!=NULL && event->getAttribute("eventId")!=NULL)
            {
                std::string module_name = event->getAttribute("module");
                simtime_t when = atof(event->getAttribute("when"));
                std::string id_str = event->getAttribute("eventId");
                if (id_str!="")
                {
                    int id_event = atoi(id_str.c_str());
                    if (repetition_id == id_event)
                    {
                        // this event is meant for this run id. schedule it
					    this->scheduleDisablePowerControlEvent(when,module_name);
                    }
				}
                else
                {
                    // no run id provided. schedule the event
                    this->scheduleDisablePowerControlEvent(when,module_name);
			    }
            }
            else
            {
                error("PowerControlEvent definition does not specify module, when or eventId: %s",event->tostr(1).c_str());
            }
        }
    }
}

void PowerControlManager::scheduleEnablePowerControlEvent(simtime_t when, std::string module_name)
{
    cModule* mod = simulation.getModuleByPath(module_name.c_str());
    if (mod!=NULL)
        this->scheduleEnablePowerControlEvent(when,mod);
    else
        EV << "EnableEvent: module " << module_name << " does not exist. ignoring" << endl;
}

void PowerControlManager::scheduleEnablePowerControlEvent(simtime_t when, cModule* mod)
{
    cMethodCallContextSwitcher __ctx(this); __ctx.methodCallSilent("scheduleEnablePowerControlEvent()");
    if (mod!=NULL)
    {
        std::stringstream  tmp;
        tmp << "Enable " << mod->getName();
        PowerControlMessage* msg = new EnableModuleMessage(tmp.str().c_str());
        msg->setModuleId(mod->getId());
        EV << "Scheduling EnablePowerControlEvent for " << mod->getFullPath() << " at " << when << endl;
        this->scheduleAt(when,msg);
    }
}

void PowerControlManager::scheduleDisablePowerControlEvent(simtime_t when, std::string module_name)
{
    cModule* mod = simulation.getModuleByPath(module_name.c_str());
    if (mod!=NULL)
        this->scheduleDisablePowerControlEvent(when,mod);
    else
        EV << "DisableEvent: module " << module_name << " does not exist. ignoring" << endl;
}

void PowerControlManager::scheduleDisablePowerControlEvent(simtime_t when, cModule* mod)
{

    cMethodCallContextSwitcher __ctx(this); __ctx.methodCallSilent("scheduleDisablePowerControlEvent()");
    if (mod!=NULL)
    {
        std::stringstream  tmp;
        tmp << "Disable " << mod->getName();
        PowerControlMessage* msg = new DisableModuleMessage(tmp.str().c_str());
        msg->setModuleId(mod->getId());
        EV << "Scheduling DisablePowerControlEvent for " << mod->getFullPath() << " at " << when << endl;
        this->scheduleAt(when,msg);
    }
}

void PowerControlManager::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage() && dynamic_cast<PowerControlMessage*>(msg))
    {
        // check if the incoming event is enable or disable
        if (dynamic_cast<EnableModuleMessage*>(msg))
            this->handleEnableModuleMessage(dynamic_cast<EnableModuleMessage*>(msg));
        else if (dynamic_cast<DisableModuleMessage*>(msg))
            this->handleDisableModuleMessage(dynamic_cast<DisableModuleMessage*>(msg));
	    else
	        EV << "Unknown type of Power control Message: it should be defined in the handleMessage method at the PowerControlManager" << endl;
    }
    else
       	EV << "Ignoring external messages" << endl;
    delete(msg);
}

void PowerControlManager::handleEnableModuleMessage(EnableModuleMessage* em)
{
    int moduleId = em->getModuleId();
    cModule* mod = simulation.getModule(moduleId);
    if (mod!=NULL)
    {
        EV << "arriving enabling event for module " << mod->getFullPath() << endl;
        mod->bubble("Enabling Device");
        this->enableModule(mod);
        mod->getDisplayString().setTagArg("i",1,"");
    }
}

void PowerControlManager::handleDisableModuleMessage(DisableModuleMessage* dm)
{
    int moduleId = dm->getModuleId();
    cModule* mod = simulation.getModule(moduleId);
    if (mod!=NULL)
    {
        EV << "arriving disabling event for module " << mod->getFullPath() << endl;
        mod->bubble("Disabling Device");
        this->disableModule(mod);
        mod->getDisplayString().setTagArg("i",1,"red");
    }
}


void PowerControlManager::enableModule(cModule* mod)
{
    if (mod->isSimple())
    {
        // simple module.
        if (dynamic_cast<IPowerControl*>(mod))
        {
            IPowerControl* ipcm = dynamic_cast<IPowerControl*>(mod);
            if (!ipcm->isEnabled())
            {
            	EV << "Enabling " << mod->getFullName() << endl;
            	cMethodCallContextSwitcher __ctx(mod); __ctx.methodCall("enableModule()");
            	ipcm->enableModule();
            }
        }
        else
			EV << "Module " << mod->getFullName() << " does not allow power control. skipping"  << endl;
    }
    else
    {
        // compound. iterate on its components
       for (cModule::SubmoduleIterator iter(mod); !iter.end(); iter++)
       {
    	   cModule* sm = iter();
    	   this->enableModule(sm);
       }
    }
}

void PowerControlManager::disableModule(cModule* mod)
{
    if (mod->isSimple())
    {
        // simple module.
        if (dynamic_cast<IPowerControl*>(mod))
        {
            IPowerControl* ipcm = dynamic_cast<IPowerControl*>(mod);
            if (ipcm->isEnabled())
            {
                EV << "Disabling " << mod->getFullName() << endl;
                cMethodCallContextSwitcher __ctx(mod); __ctx.methodCall("disableModule()");
                ipcm->disableModule();
            }
		}
        else
            EV << "Module " << mod->getFullName() << " does not allow power control. skipping"  << endl;
    }
    else
    {
        // compound. iterate on its components
        for (cModule::SubmoduleIterator iter(mod); !iter.end(); iter++)
        {
            cModule* sm = iter();
            this->disableModule(sm);
        }
    }
}

