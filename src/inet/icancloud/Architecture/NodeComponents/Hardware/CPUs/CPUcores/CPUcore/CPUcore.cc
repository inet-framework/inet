#include "CPUcore.h"

namespace inet {

namespace icancloud {

#define ZERO_DOUBLE 0.000000F

Define_Module (CPUcore);

CPUcore::~CPUcore(){

}

void CPUcore::initialize(int stage){

    ICore::initialize(stage);
    if (stage == INITSTAGE_PHYSICAL_OBJECT_CACHE)
    {
        std::ostringstream osStream;

	    // Set the moduleIdName
	    osStream << "CPUcore." << getId();
	    moduleIdName = osStream.str();
	    
	    // Get the energy behaviour of cores
	    independentCores = getParentModule()-> par("indepentCores").boolValue();

	    // Get the speed parameter
	    speed = par ("speed").intValue();
	    tick_s = par ("tick_s").doubleValue();

	    // Calculate the ipt
	    calculateIPT(speed);

	    // Gate IDs
	    inGate = gate ("in");
	    outGate = gate ("out");
	    
	    showStartedModule ("CPU Core Speed:%d MIPS.  Tick:%f s.  IPT:%d",
			    			speed,
			    			tick_s.dbl(),
			    			ipt);
    }

}


void CPUcore::finish(){

    // Finish the super-class
    HWEnergyInterface::finish();
}


cGate* CPUcore::getOutGate (cMessage *msg){

	// If msg arrive from scheduler
	if (msg->getArrivalGate()==inGate){
		if (outGate->getNextGate()->isConnected()){
			return (outGate);
		}
		else {
			showErrorMessage("E_CPUcore: nextGate to outGate not connected");
		}
	}
	else {
		showErrorMessage("E_CPUcore: arrival gate different to inGate");
	}

	// If gate not found!
	return nullptr;
}


void CPUcore::processSelfMessage (cMessage *msg){

			// Latency message...
		if (!strcmp (msg->getName(), SM_LATENCY_MESSAGE.c_str())){

			// Cast!
		    auto pkt = check_and_cast<Packet *> (pendingMessage);
		    const auto &sm_cpu = pkt->peekAtFront<icancloud_App_CPU_Message>();
		    if (sm_cpu == nullptr)
		        throw cRuntimeError("Incorrect header");
	
		    pkt->trim();
			// Init pending message...
			pendingMessage = nullptr;
	
			// establish is a response msg
			pkt->trimFront();
			auto &sm_cpu_aux = pkt->removeAtFront<icancloud_App_CPU_Message>();
			sm_cpu_aux->setIsResponse(true);
			pkt->insertAtFront(sm_cpu_aux);

			// Send message back!
			sendResponseMessage (pkt);

			// Reset the time control
            currentTime = 0;

            // Deactivate the flag
            executingMessage = false;

		}

		else if (!strcmp (msg->getName(), "continueExecuting")){

            auto pkt = check_and_cast<Packet *> (pendingMessage);
            const auto &msgToExecute = pkt->peekAtFront<icancloud_Message>();

            if (msgToExecute == nullptr)
                throw cRuntimeError("Incorrect header");

            // Init pending message...
            pendingMessage = nullptr;

            // Active the flag of continue executing
            executingMessage = true;

		    // Process the executing message
		    processRequestMessage(pkt);
		}

		else
			showErrorMessage ("Unknown self message [%s]", msg->getName());	

		cancelAndDelete(msg);
}


void CPUcore::processRequestMessage (Packet *pkt){

	simtime_t cpuTime = 0;

	string nodeState;
	int operation;
	bool executionComplete ;
	
	// Initialize
	executionComplete = true;
	// Casting to debug!
    const auto &sm_cpu = pkt->peekAtFront<icancloud_App_CPU_Message>();
    if (sm_cpu == nullptr)
        throw cRuntimeError("Incorrect header");

	operation = sm_cpu->getOperation();

	if (operation != SM_CHANGE_CPU_STATE){

	    if (actualPState == 0){
	        throw cRuntimeError("The cpu core has received a message to compute and it is in off state");
	    }

		if (pendingMessage != nullptr)
			showErrorMessage ("Core currently processing another computing block!");		
		
		cpuTime = sm_cpu->getCpuTime();

		// Link pending message
		pendingMessage = pkt;
		
		// Uses time
		if (cpuTime > 0.0){

			// Executes completely the CB
			if ( (cpuTime > 0.0) && (cpuTime <= 0.1) ){
				changeState (C0_P11);
			}
			else if ( (cpuTime > 0.0) && (cpuTime <= 0.2) ){
				changeState (C0_P10);
			}
			else if ( (cpuTime > 0.0) && (cpuTime <= 0.3) ){
				changeState (C0_P9);
			}
			else if ( (cpuTime > 0.0) && (cpuTime <= 0.4) ){
				changeState (C0_P7);
			}
			else if ( (cpuTime > 0.0) && (cpuTime <= 0.5) ){
				changeState (C0_P6);
			}
			else if ( (cpuTime > 0.0) && (cpuTime <= 0.6) ){
				changeState (C0_P5);
			}
			else if ( (cpuTime > 0.0) && (cpuTime <= 0.7) ){
				changeState (C0_P4);
			}
			else if ( (cpuTime > 0.0) && (cpuTime <= 0.8) ){
				changeState (C0_P2);
			}
			else if ( (cpuTime > 0.0) && (cpuTime <= 0.9) ){
				changeState (C0_P1);
			}
			else if ( (cpuTime > 0.0) && (cpuTime <= 1.0) ){
				changeState (C0_P0);
			}

			if ((sm_cpu->getQuantum() == INFINITE_QUANTUM) ||
			   ((sm_cpu->getQuantum() * tick_s.dbl()) > ((sm_cpu->getCpuTime()).dbl()))){
				cpuTime = getMaximumTimeToExecute (sm_cpu->getCpuTime());
			}
			
			// Executes a slice
	        // TODO: The cpu scheduler should increase the speed of the cpu depending on the calculated load..
			else{
				cpuTime = tick_s.dbl() * sm_cpu->getQuantum();
				pkt->trimFront();
				auto sm_cpu = pkt->removeAtFront<icancloud_App_CPU_Message>();
				sm_cpu->executeTime (cpuTime);
				pkt->insertAtFront(sm_cpu);
			}
		}

		// Execute current computing block completely.

		else{

			if ((sm_cpu->getQuantum() == INFINITE_QUANTUM) || (sm_cpu->getQuantum() >= ceil (sm_cpu->getRemainingMIs()/ipt))){

			    if (!executingMessage){

			        currentTime = getTimeToExecuteCompletely (sm_cpu->getRemainingMIs());
			        pkt->trimFront();
			        auto sm_cpu = pkt->removeAtFront<icancloud_App_CPU_Message>();
			        sm_cpu->executeMIs (sm_cpu->getRemainingMIs());
			        pkt->insertAtFront(sm_cpu);
			    }

                if (currentTime <= 1){
                      executionComplete = true;
                }
                // 1 is a factor that should be adjusted depending on the fast than the dvfs increases!
                else{
                    changeDeviceState(INCREMENT_SPEED);
                    currentTime = currentTime - 1;
                    executionComplete = false;
                }
                cpuTime = currentTime;
			}

			// Execute the corresponding number of ticks
			else{
	            // TODO: The cpu scheduler should increase the speed of the cpu depending on the calculated load..
				cpuTime = tick_s.dbl() * sm_cpu->getQuantum();
				pkt->trimFront();
				auto sm_cpu = pkt->removeAtFront<icancloud_App_CPU_Message>();
				sm_cpu->executeMIs (ipt * sm_cpu->getQuantum());
				pkt->insertAtFront(sm_cpu);
			}
		}
		
		if (DEBUG_CPUcore) {
	        auto sm_cpu = pkt->removeAtFront<icancloud_App_CPU_Message>();
	    	showDebugMessage ("[%s] Processing Cb. Computing time:%f - %s",
	    						moduleIdName.c_str(),
	    						cpuTime.dbl(),
	    						sm_cpu->contentsToString(DEBUG_MSG_CPUcore).c_str());
	    	pkt->insertAtFront(sm_cpu);
		}
		
		if (executionComplete)	{
		    cMessage* message = new cMessage (SM_LATENCY_MESSAGE.c_str());
		    scheduleAt (simTime()+cpuTime, message);
		}
		else {
		    cMessage* message = new cMessage ("continueExecuting");
		    scheduleAt (simTime()+cpuTime, message);
		}

	}
	else {
		// The node is changing the state!
	    pkt->trimFront();
	    auto sm_cpu = pkt->removeAtFront<icancloud_App_CPU_Message>();
		nodeState = sm_cpu->getChangingState();
		pkt->insertAtFront(sm_cpu);

		changeDeviceState (nodeState.c_str());

		delete(pkt);
	}

}


void CPUcore::processResponseMessage (Packet *){
	showErrorMessage ("This module cannot receive response messages!");
}


simtime_t CPUcore::getTimeToExecuteCompletely (long int remainingMIs){
	
	simtime_t cpuTime;
	double instructionsPerSecond = ((1/tick_s.dbl())*ipt);

	cpuTime = ((remainingMIs/instructionsPerSecond));

	return cpuTime;

}


simtime_t CPUcore::getMaximumTimeToExecute (simtime_t remainingTime){
	
	simtime_t cpuTime;

		cpuTime = ((int) ceil (remainingTime.dbl() / (tick_s.dbl()))) * (tick_s.dbl());

	return cpuTime;
}


void CPUcore::changeDeviceState (const string & state, unsigned componentIndex){

	string pstate;
	int pstatePosition = actualPState;

	string const PStates[] = { "off", "c0_p11", "c0_p10", "c0_p9", "c0_p8", "c0_p7", "c0_p6", "c0_p5", "c0_p4", "c0_p3", "c0_p2", "c0_p1", "c0_p0" };
	if (state == MACHINE_STATE_IDLE ) {

		nodeState = MACHINE_STATE_IDLE;
		changeState (C0_P11);

	} else if (state == MACHINE_STATE_RUNNING ) {

		nodeState = MACHINE_STATE_RUNNING;
		changeState (C0_P10);

	} else if (state == MACHINE_STATE_OFF ) {

		nodeState = MACHINE_STATE_OFF;
		changeState (OFF);

	} else if (state == INCREMENT_SPEED) {

		nodeState = MACHINE_STATE_RUNNING;
		if (pstatePosition < 12){
		    pstatePosition++;
			pstate = PStates[pstatePosition++];
			changeState (pstate);
		}

	} else if (state == DECREMENT_SPEED ) {

		nodeState = MACHINE_STATE_RUNNING;
		if (pstatePosition > 1){
		    pstatePosition--;
			pstate = PStates[pstatePosition--];
			changeState (pstate);
		}
	} else  {

        nodeState = MACHINE_STATE_OFF;
        changeState (OFF);
	}
}


void CPUcore::changeState (const string & energyState, unsigned componentIndex){

	double relation_speed;
	int64_t min_speed = 0;
	int statesSize = (e_getStatesSize()-1);

	// calculate the relation_speed;
	min_speed = speed - (speed * 0.6);
	relation_speed = ( (speed - min_speed) / statesSize);

	// The cpu is completely off.
	if (energyState == OFF){
		actualPState = 0;
		current_speed = 0;
	}

	// P-state 0 - 2.40 GHz
	else if (energyState == C0_P0) {
		current_speed = speed;
		actualPState = 12;
	}

	// P-state 1 - 2.27 GHz
	else if (energyState == C0_P1) {
	    current_speed = (speed - relation_speed);
		actualPState = 11;
	}

	// P-state 2 - 2.13 GHz
	else if (energyState == C0_P2) {
		current_speed = (speed - (relation_speed * 2));
		actualPState = 10;
	}

	// P-state 3 - 2.00 GHz
	else if (energyState == C0_P3) {
		current_speed = (speed -(relation_speed * 3));
		actualPState = 9;
	}

	// P-state 4 - 1.87 GHz
	else if (energyState  == C0_P4) {
		current_speed = (speed -(relation_speed * 4));
		actualPState = 8;
	}

	// P-state 5 - 1.73 GHz
	else if (energyState == C0_P5) {
		current_speed = (speed -(relation_speed * 5));
		actualPState = 7;
	}

	// P-state 6 - 1.60 GHz
	else if (energyState == C0_P6) {
		current_speed = (speed -(relation_speed * 6));
		actualPState = 6;
	}

	// P-state 7 - 1.47 GHz
 	else if (energyState == C0_P7) {
 		current_speed = (speed -(relation_speed * 7));
 		actualPState = 5;
 	}

	// P-state 8 - 1.33 GHz
 	else if (energyState == C0_P8){
 		current_speed = (speed -(relation_speed * 8));
 		actualPState = 4;
 	}

	// P-state 9 - 1.20 GHz
 	else if (energyState == C0_P9){
 		current_speed = (speed -(relation_speed * 9));
 		actualPState = 3;
 	}

	// P-state 10 - 1.07 GHz
 	else if (energyState == C0_P10){
 		current_speed = (speed -(relation_speed * 10));
 		actualPState = 2;
 	}

	// P-state 11 - 933 GHz
 	else if (energyState == C0_P11){
 		current_speed = (speed -(relation_speed * 11));
 		actualPState = 1;
 	}

//	// Stops CPU main internal clocks via software; bus interface unit and APIC are kept running at full speed.
// 	else if (strcmp (energyState.c_str(),C1_Halt) == 0){
//
// 	}
//
//	// Stops CPU main internal clocks via hardware and reduces CPU voltage; bus interface unit and APIC are kept running at full speed.
// 	else if (strcmp (energyState.c_str(),C2_StopGrant) == 0){
//
// 	}
//
//	// Stops all CPU internal and external clocks
// 	else if (strcmp (energyState.c_str(),C3_Sleep) == 0){
//
// 	}
//
//	// Reduces CPU voltage
// 	else if (strcmp (energyState.c_str(),C4_DeeperSleep) == 0){
//
// 	}
//
//	// Reduces CPU voltage even more and turns off the memory cache
// 	else if (strcmp (energyState.c_str(),C5_EnhancedDeeperSleep) == 0){
//
// 	}
//
// 	else {
//
// 	}

	calculateIPT(current_speed);
	e_changeState (energyState);

}

} // namespace icancloud
} // namespace inet
