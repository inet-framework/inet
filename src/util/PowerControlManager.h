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

#ifndef __INETMANET_POWERCONTROLMANAGER_H_
#define __INETMANET_POWERCONTROLMANAGER_H_

#include <omnetpp.h>
#include "PowerControlMessages_m.h"

class IPowerControl;

/**
 * Power Control Manager
 * used to enable or disable modules that implements IPowerControl Interface
 */
class PowerControlManager : public cSimpleModule
{
  private:
    virtual void handleEnableModuleMessage(EnableModuleMessage* em);
    virtual void handleDisableModuleMessage(DisableModuleMessage* dm);
  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
  public:
    static PowerControlManager* get();

    // by name
    virtual void scheduleEnablePowerControlEvent(simtime_t when, std::string module_name);
    virtual void scheduleDisablePowerControlEvent(simtime_t when, std::string module_name);

    // by module ptr
    virtual void scheduleEnablePowerControlEvent(simtime_t when, cModule* module);
    virtual void scheduleDisablePowerControlEvent(simtime_t when, cModule* module);

    virtual void enableModule(cModule* mod);
    virtual void disableModule(cModule* mod);
};

#endif
