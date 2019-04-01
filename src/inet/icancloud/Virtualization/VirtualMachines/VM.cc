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

#include "inet/icancloud/Virtualization/VirtualMachines/VM.h"

namespace inet {

namespace icancloud {


Define_Module(VM);


VM::~VM() {
    states_log.clear();

}

void VM::initialize(int stage) {

    Machine::initialize(stage);
    // Define ..
    if (stage == INITSTAGE_LOCAL) {
        cModule* osMod;
        string vmTypeName;

        // Init ..
        states_log.clear();
        pending_operation = NOT_PENDING_OPS;
        userID = -1;
        nodeName = -1;
        nodeSetName = "";
        vmName = "";

        osMod = getSubmodule("osModule")->getSubmodule("syscallManager");
        os = dynamic_cast<VMSyscallManager*>(osMod);
        Machine::changeState(MACHINE_STATE_OFF);
    }
}

void VM::finish(){

    Machine::finish();
}

void VM::changeState(string newState){
    vmStatesLog_t* log;
    string oldstate;

    if (states_log.size() == 0 ){

        log = new vmStatesLog_t();
        log->vm_state = newState;
        log->init_time_M = simTime().dbl() / 60;

        states_log.push_back(log);
    } else {
       oldstate = (*(states_log.end() -1))->vm_state;

       if (strcmp(oldstate.c_str(), newState.c_str()) != 0){
           log = new vmStatesLog_t();
           log->vm_state = newState;
           log->init_time_M = simTime().dbl() / 60;

           states_log.push_back(log);
       }
    }

    Machine::changeState(newState);
};


void VM::shutdownVM (){

	// Define ..
	//	vector<int>::iterator jobIt;

	// Begin ..
	if ((!equalStates(getState(),MACHINE_STATE_OFF) || (!(equalStates(getState(),MACHINE_STATE_IDLE))))){

	    os->removeAllProcesses();
	}

	if (!equalStates(getState(),MACHINE_STATE_OFF)){
	    Machine::changeState(MACHINE_STATE_OFF);
	}

	 pending_operation = NOT_PENDING_OPS;

}

bool VM::isAppRunning(int pId){


	return os->isAppRunning(pId);
}

void VM::setManager(icancloud_Base* manager){
    Machine::setManager(manager);
    os->setManager(manager);
}

} // namespace icancloud
} // namespace inet
